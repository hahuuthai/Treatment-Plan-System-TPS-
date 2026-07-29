#include "RTDoseLoader.h"

#include <dcmtk/dcmdata/dctk.h>
#include <QString>
#include <QDebug>
#include <cmath>


bool RTDoseLoader::loadDose(
    const std::string& fileName,
    DoseGridModel& dose)
{
    dose.clear();

    DcmFileFormat file;

    if (!file.loadFile(fileName.c_str()).good())
        return false;

    DcmDataset* ds =
        file.getDataset();

    //---------------------------------------------------
    // Check modality
    //---------------------------------------------------

    OFString modality;

    ds->findAndGetOFString(
        DCM_Modality,
        modality);

    if (modality != "RTDOSE")
        return false;

    //---------------------------------------------------
    // Size
    //---------------------------------------------------

    Uint16 rows = 0;
    Uint16 cols = 0;

    ds->findAndGetUint16(
        DCM_Rows,
        rows);

    ds->findAndGetUint16(
        DCM_Columns,
        cols);

    OFString frameStr;

    int numberOfFrames = 1;

    if (ds->findAndGetOFString(
        DCM_NumberOfFrames,
        frameStr).good())
    {
        numberOfFrames =
            QString(frameStr.c_str()).toInt();
    }

    // Rows/Columns/NumberOfFrames of 0 means the dataset is
    // missing mandatory RTDOSE geometry tags - resizing to a
    // degenerate grid would silently produce an "empty but
    // valid-looking" DoseGridModel further down the pipeline.
    if (rows == 0 || cols == 0 || numberOfFrames <= 0)
    {
        qDebug()
            << "RTDoseLoader: invalid grid dimensions"
            << "Rows =" << rows
            << "Columns =" << cols
            << "Frames =" << numberOfFrames;

        return false;
    }

    // Columns = X extent (width), Rows = Y extent (height) per DICOM.
    dose.resize(
        cols,
        rows,
        numberOfFrames);

    //---------------------------------------------------
    // Origin
    //---------------------------------------------------

    for (int i = 0; i < 3; i++)
    {
        ds->findAndGetFloat64(
            DCM_ImagePositionPatient,
            dose.origin[i],
            i);
    }

    //---------------------------------------------------
    // Image Orientation
    //---------------------------------------------------

    for (int i = 0; i < 6; i++)
    {
        ds->findAndGetFloat64(
            DCM_ImageOrientationPatient,
            dose.direction[i],
            i);
    }

    //---------------------------------------------------
    // Pixel spacing
    //
    // DICOM PixelSpacing = [row spacing, column spacing]:
    //   spacing[0] = row spacing    (distance between rows)
    //   spacing[1] = column spacing (distance between columns)
    // Stored as-is here; CoordinateAlignment is responsible
    // for pairing each value with the correct direction cosine.
    //---------------------------------------------------

    ds->findAndGetFloat64(
        DCM_PixelSpacing,
        dose.spacing[0],
        0);

    ds->findAndGetFloat64(
        DCM_PixelSpacing,
        dose.spacing[1],
        1);

    //---------------------------------------------------
    // Z spacing / GridFrameOffsetVector
    //---------------------------------------------------

    DcmElement* elem = nullptr;

    if (ds->findAndGetElement(
        DcmTagKey(0x3004, 0x000C),
        elem).good())
    {
        unsigned long vm = elem->getVM();

        dose.frameOffsets.resize(vm);

        for (unsigned long i = 0; i < vm; i++)
        {
            Float64 value = 0.0;

            elem->getFloat64(value, i);

            dose.frameOffsets[i] = value;
        }
    }

    // dose.spacing[2] is used as the fallback Z step whenever
    // frameOffsets is empty (e.g. single-frame dose, or a
    // non-conformant file missing GridFrameOffsetVector). It is
    // never populated from any DICOM tag otherwise, so without
    // this it silently stays at the DoseGridModel default of
    // 1.0 mm regardless of the actual slice spacing.
    if (dose.frameOffsets.size() >= 2)
    {
        double span =
            dose.frameOffsets.back() -
            dose.frameOffsets.front();

        double avgStep =
            span / double(dose.frameOffsets.size() - 1);

        if (std::abs(avgStep) > 1e-6)
        {
            dose.spacing[2] = avgStep;
        }
    }
    else
    {
        Float64 sliceThickness = 0.0;

        if (ds->findAndGetFloat64(
            DCM_SliceThickness,
            sliceThickness).good()
            && sliceThickness > 1e-6)
        {
            dose.spacing[2] = sliceThickness;
        }
        else
        {
            qDebug()
                << "RTDoseLoader: no GridFrameOffsetVector or "
                "SliceThickness found, falling back to"
                << dose.spacing[2]
                << "mm Z spacing";
        }
    }

    //---------------------------------------------------
    // Dose information
    //---------------------------------------------------

    OFString unit;
    OFString type;
    OFString summation;

    ds->findAndGetOFString(
        DCM_DoseUnits,
        unit);

    ds->findAndGetOFString(
        DCM_DoseType,
        type);

    ds->findAndGetOFString(
        DCM_DoseSummationType,
        summation);

    dose.doseUnit =
        unit.c_str();

    dose.doseType =
        type.c_str();

    dose.doseSummationType =
        summation.c_str();

    //---------------------------------------------------
    // Frame Of Reference UID
    //---------------------------------------------------

    OFString uid;

    ds->findAndGetOFString(
        DCM_FrameOfReferenceUID,
        uid);

    dose.frameOfReferenceUID =
        uid.c_str();

    //---------------------------------------------------
    // Study UID
    //---------------------------------------------------

    ds->findAndGetOFString(
        DCM_StudyInstanceUID,
        uid);

    dose.studyInstanceUID =
        uid.c_str();

    //---------------------------------------------------
    // Series UID
    //---------------------------------------------------

    ds->findAndGetOFString(
        DCM_SeriesInstanceUID,
        uid);

    dose.seriesInstanceUID =
        uid.c_str();

    //---------------------------------------------------
    // SOP UID
    //---------------------------------------------------

    ds->findAndGetOFString(
        DCM_SOPInstanceUID,
        uid);

    dose.sopInstanceUID =
        uid.c_str();

    //---------------------------------------------------
    // Scaling
    //---------------------------------------------------

    Float64 scaling = 1.0;

    ds->findAndGetFloat64(
        DCM_DoseGridScaling,
        scaling);

    dose.doseGridScaling =
        scaling;

    //---------------------------------------------------
    // Pixel Data
    //
    // RTDOSE pixel data can be 16-bit or 32-bit unsigned
    // integers depending on BitsAllocated. Reading a 32-bit
    // dataset through findAndGetUint16Array would silently
    // misinterpret the byte layout, so branch on the tag
    // rather than assuming 16-bit.
    //---------------------------------------------------

    Uint16 bitsAllocated = 16;

    ds->findAndGetUint16(
        DCM_BitsAllocated,
        bitsAllocated);

    const size_t voxelCount =
        static_cast<size_t>(dose.width) *
        static_cast<size_t>(dose.height) *
        static_cast<size_t>(dose.depth);

    if (bitsAllocated == 32)
    {
        const Uint32* pixelData32 = nullptr;

        if (!ds->findAndGetUint32Array(
            DCM_PixelData,
            pixelData32).good()
            || !pixelData32)
        {
            return false;
        }

        unsigned long count = 0;

        if (ds->findAndGetElement(DCM_PixelData, elem).good())
        {
            count =
                elem->getLength() / sizeof(Uint32);
        }

        if (count < voxelCount)
        {
            qDebug()
                << "RTDoseLoader: PixelData has"
                << count
                << "elements, expected at least"
                << voxelCount;

            return false;
        }

        for (size_t i = 0; i < voxelCount; i++)
        {
            dose.dose[i] =
                static_cast<float>(
                    pixelData32[i] * scaling);
        }
    }
    else
    {
        const Uint16* pixelData = nullptr;

        if (!ds->findAndGetUint16Array(
            DCM_PixelData,
            pixelData).good()
            || !pixelData)
        {
            return false;
        }

        unsigned long count = 0;

        if (ds->findAndGetElement(DCM_PixelData, elem).good())
        {
            count =
                elem->getLength() / sizeof(Uint16);
        }

        if (count < voxelCount)
        {
            qDebug()
                << "RTDoseLoader: PixelData has"
                << count
                << "elements, expected at least"
                << voxelCount;

            return false;
        }

        for (size_t i = 0; i < voxelCount; i++)
        {
            dose.dose[i] =
                static_cast<float>(
                    pixelData[i] * scaling);
        }
    }

    return true;
}