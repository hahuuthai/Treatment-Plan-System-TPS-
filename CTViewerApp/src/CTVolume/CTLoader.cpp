#include "CTLoader.h"

#include <dcmtk/dcmdata/dctk.h>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <QMessageBox>

namespace fs = std::filesystem;

// =============================
// SCAN FOLDER
// =============================
bool CTLoader::scanFolder(
    const std::string& folder,
    std::map<std::string, Patient>& patients,
    ProgressCallback progress)
{
    std::vector<fs::path> files;

    for (const auto& entry : fs::recursive_directory_iterator(folder))
    {
        if (entry.is_regular_file())
            files.push_back(entry.path());
    }

    int total = (int)files.size();
    int current = 0;

    // ===== FIX: CHECK DICOM =====
    bool hasValidDicom = false;

    for (const auto& path : files)
    {
        current++;

        DcmFileFormat file;
        if (!file.loadFile(path.string().c_str()).good())
        {
            if (progress)
                progress(current, total, "Scanning DICOM...");
            continue;
        }

        DcmDataset* ds = file.getDataset();
        if (!ds)
            continue;

        // =============================
        // BASIC TAGS
        // =============================
        OFString patientName, seriesUID, seriesDesc;
        OFString modality, studyDate;
        OFString patientID, studyDesc, seriesNumber;
        OFString rescaleSlopeStr, rescaleInterceptStr;
        OFString frameUID;

        ds->findAndGetOFString(DCM_PatientName, patientName);
        ds->findAndGetOFString(DCM_SeriesInstanceUID, seriesUID);
        ds->findAndGetOFString(DCM_SeriesDescription, seriesDesc);
        ds->findAndGetOFString(DCM_Modality, modality);
        ds->findAndGetOFString(DCM_StudyDate, studyDate);
        ds->findAndGetOFString(DCM_PatientID, patientID);
        ds->findAndGetOFString(DCM_StudyDescription, studyDesc);
        ds->findAndGetOFString(DCM_SeriesNumber, seriesNumber);
        ds->findAndGetOFString(DCM_RescaleSlope, rescaleSlopeStr);
        ds->findAndGetOFString(DCM_RescaleIntercept, rescaleInterceptStr);
        ds->findAndGetOFString(
            DCM_FrameOfReferenceUID,
            frameUID);

        // ===== ĐÁNH DẤU CÓ DICOM HỢP LỆ =====
        hasValidDicom = true;

        std::string pName = patientName.empty() ? "Unknown" : patientName.c_str();
        std::string sUID = seriesUID.empty() ? "Unknown" : seriesUID.c_str();

        auto& patient = patients[pName];
        patient.name = pName;

        auto& series = patient.seriesMap[sUID];
        series.patientName = pName;
        series.seriesUID = sUID;
        series.description = seriesDesc.c_str();
        series.modality = modality.c_str();
        series.studyDate = studyDate.c_str();

        series.patientID = patientID.c_str();
        series.studyDescription = studyDesc.c_str();
        series.seriesNumber = seriesNumber.c_str();
        series.frameOfReferenceUID =
            frameUID.c_str();

        if (!rescaleSlopeStr.empty())
            series.rescaleSlope = atof(rescaleSlopeStr.c_str());

        if (!rescaleInterceptStr.empty())
            series.rescaleIntercept = atof(rescaleInterceptStr.c_str());

        // =============================
        // IMAGE POSITION PATIENT
        // =============================
        double ipp[3] = { 0,0,0 };

        for (int i = 0; i < 3; i++)
        {
            ds->findAndGetFloat64(
                DCM_ImagePositionPatient,
                ipp[i],
                i);
        }



        // =============================
        // SPACING
        // =============================
        ds->findAndGetFloat64(DCM_PixelSpacing, series.spacing[0], 0);
        ds->findAndGetFloat64(DCM_PixelSpacing, series.spacing[1], 1);
        ds->findAndGetFloat64(DCM_SliceThickness, series.spacing[2]);

        series.sliceThickness = series.spacing[2];

        // =============================
        // ORIENTATION
        // =============================
        for (int i = 0; i < 6; i++)
        {
            if (!ds->findAndGetFloat64(DCM_ImageOrientationPatient, series.direction[i], i).good())
                series.direction[i] = 0;
        }

        double row[3] =
        {
            series.direction[0],
            series.direction[1],
            series.direction[2]
        };

        double col[3] =
        {
            series.direction[3],
            series.direction[4],
            series.direction[5]
        };

        double normal[3];

        normal[0] =
            row[1] * col[2]
            - row[2] * col[1];

        normal[1] =
            row[2] * col[0]
            - row[0] * col[2];

        normal[2] =
            row[0] * col[1]
            - row[1] * col[0];

        double location =
            ipp[0] * normal[0]
            + ipp[1] * normal[1]
            + ipp[2] * normal[2];

        // fallback orientation
        bool invalidOrientation = true;
        for (int i = 0; i < 6; i++)
        {
            if (series.direction[i] != 0)
            {
                invalidOrientation = false;
                break;
            }
        }

        if (invalidOrientation)
        {
            series.direction[0] = 1; series.direction[1] = 0; series.direction[2] = 0;
            series.direction[3] = 0; series.direction[4] = 1; series.direction[5] = 0;
        }

        // =============================
        // ORIGIN 
        // =============================
        if (series.slices.empty())
        {
            for (int i = 0; i < 3; i++)
            {
                if (!ds->findAndGetFloat64(DCM_ImagePositionPatient, series.origin[i], i).good())
                    series.origin[i] = 0;
            }
        }

        // =============================
        // ADD SLICE
        // =============================
        Slice s;

        s.path = path.string();

        s.position[0] = ipp[0];
        s.position[1] = ipp[1];
        s.position[2] = ipp[2];

        s.location = location;

        series.slices.push_back(s);

        if (progress)
            progress(current, total, "Scanning DICOM...");
    }

    // ===== KHÔNG CÓ DICOM =====
    if (!hasValidDicom)
    {
        QMessageBox::warning(
            nullptr,
            "No DICOM Found",
            "No valid CT DICOM data found in the selected folder."
        );
        return false;
    }

    return !patients.empty();
}


// =============================
// BUILD VOLUME
// =============================
bool CTLoader::buildVolume(const Series& series, CTVolume& volume)
{
    if (series.slices.empty()) return false;

    volume.clear();

    auto slices = series.slices;

    std::sort(
        slices.begin(),
        slices.end(),
        [](const Slice& a,
            const Slice& b)
        {
          
            return a.location < b.location;
        });

    // ======================================
    // CHECK Z-SPACING ONLY
    // DO NOT REMOVE SLICE
    // ======================================

    for (size_t i = 1; i + 1 < slices.size(); i++)
    {
        double prevZ = slices[i - 1].location;
        double currZ = slices[i].location;
        double nextZ = slices[i + 1].location;

        double dz1 = std::abs(prevZ - currZ);
        double dz2 = std::abs(currZ - nextZ);

        if (std::abs(dz1 - dz2) > 0.5)
        {
            qDebug()
                << "WARNING BAD Z SPACING:"
                << currZ
                << QString::fromStdString(slices[i].path);
        }
    }

    qDebug() << "===== SORTED SLICES =====";

    for (int i = 0; i < slices.size(); i++)
    {
        qDebug()
            << i
            << "Location ="
            << slices[i].location
            << "IPP ="
            << slices[i].position[0]
            << slices[i].position[1]
            << slices[i].position[2]
            << QString::fromStdString(
                slices[i].path);
    }

    // ===== LOAD FIRST SLICE =====
    DcmFileFormat file;
    if (!file.loadFile(slices[0].path.c_str()).good())
        return false;

    DcmDataset* ds = file.getDataset();

    Uint16 rows = 0, cols = 0;
    ds->findAndGetUint16(DCM_Rows, rows);
    ds->findAndGetUint16(DCM_Columns, cols);

    volume.width = cols;
    volume.height = rows;
    volume.depth = (int)slices.size();

    // ===== MODALITY =====
    volume.modality = series.modality;

    // ===== FRAME OF REFERENCE UID =====

    OFString frameUID;

    ds->findAndGetOFString(
        DCM_FrameOfReferenceUID,
        frameUID);

    volume.frameOfReferenceUID =
        frameUID.c_str();

    // ===== SPACING =====
    ds->findAndGetFloat64(DCM_PixelSpacing, volume.spacing[0], 0);
    ds->findAndGetFloat64(DCM_PixelSpacing, volume.spacing[1], 1);
    ds->findAndGetFloat64(DCM_SliceThickness, volume.spacing[2]);

    // ===== ORIGIN =====
    for (int i = 0; i < 3; i++)
        ds->findAndGetFloat64(DCM_ImagePositionPatient, volume.origin[i], i);

    // ===== ORIENTATION =====
    for (int i = 0; i < 6; i++)
        ds->findAndGetFloat64(DCM_ImageOrientationPatient, volume.direction[i], i);

    qDebug() << "========== CT INFO ==========";

    qDebug() << "Origin :"
        << volume.origin[0]
        << volume.origin[1]
        << volume.origin[2];

    qDebug() << "Spacing :"
        << volume.spacing[0]
        << volume.spacing[1]
        << volume.spacing[2];

    qDebug() << "Direction :"
        << volume.direction[0]
        << volume.direction[1]
        << volume.direction[2]
        << volume.direction[3]
        << volume.direction[4]
        << volume.direction[5];

    qDebug() << "=============================";

    // ===== RESCALE (CHỈ DÙNG CHO CT) =====
    ds->findAndGetFloat64(DCM_RescaleSlope, volume.rescaleSlope);
    ds->findAndGetFloat64(DCM_RescaleIntercept, volume.rescaleIntercept);

    bool isCT = (volume.modality == "CT");

    // ===== ALLOC =====
    int sliceSize = volume.width * volume.height;
    volume.voxels.resize(sliceSize * volume.depth);
    volume.slicePositions.assign(
        volume.depth,
        std::array<double, 3>{ 0, 0, 0 });

    // ===== LOAD SLICES =====
    for (int z = 0; z < volume.depth; z++)
    {
        DcmFileFormat f;
        if (!f.loadFile(slices[z].path.c_str()).good())
            continue;

        DcmDataset* d = f.getDataset();

        // ===== POSITION =====
        std::array<double, 3> pos = { 0,0,0 };
        for (int i = 0; i < 3; i++)
        {
            if (!d->findAndGetFloat64(DCM_ImagePositionPatient, pos[i], i).good())
                pos[i] = volume.origin[i];
        }
        volume.slicePositions[z] = pos;

        // ===== PIXEL =====
        const Uint16* pixelData = nullptr;
        if (!d->findAndGetUint16Array(DCM_PixelData, pixelData).good() || !pixelData)
            continue;

        DcmElement* pixelElem = nullptr;

        unsigned long pixelCount = 0;

        if (d->findAndGetElement(DCM_PixelData, pixelElem).good() && pixelElem)
        {
            pixelCount =
                pixelElem->getLength() / sizeof(Uint16);
        }

        if (pixelCount < static_cast<unsigned long>(sliceSize))
        {
           
            continue;
        }

        for (int i = 0; i < sliceSize; i++)
        {
            float value = pixelData[i];


            if (isCT)
                value = value * volume.rescaleSlope + volume.rescaleIntercept;

            volume.voxels[z * sliceSize + i] = value;
        }
    }

    return true;
}