#include "RTStructLoader.h"

#include <QDebug>
#include <QColor>

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmrt/drtstrct.h>

#include "CTVolume/CTVolume.h"

// =============================================
// LOAD
// =============================================

bool RTStructLoader::load(
    const std::string& filePath,
    const CTVolume* volume,
    StructureSetContainer& structureSet)
{
    structureSet.clear();

    DcmFileFormat file;

    OFCondition status =
        file.loadFile(filePath.c_str());

    if (!status.good())
    {
        qDebug() << "Cannot load RTSTRUCT";
        return false;
    }

    DRTStructureSetIOD rtStruct;

    status =
        rtStruct.read(*file.getDataset());

    if (!status.good())
    {
        qDebug() << "Cannot parse RTSTRUCT";
        return false;
    }

    if (volume)
    {
        bool validFOR =
            validateFrameOfReference(
                file.getDataset(),
                volume);

        if (!validFOR)
        {
            qDebug() << "Frame Of Reference mismatch";
            return false;
        }
    }

    if (!parseStructures(rtStruct, structureSet))
        return false;

    if (!parseContours(rtStruct, structureSet))
        return false;

    if (!parseROIObservations(
        rtStruct,
        structureSet))
        return false;

    return true;
}

// =============================================
// VALIDATE FOR UID
// =============================================
bool RTStructLoader::validateFrameOfReference(
    DcmDataset* ds,
    const CTVolume* volume)
{
    if (!volume)
        return true;

    DcmSequenceOfItems* refFrameSeq = nullptr;

    OFCondition status =
        ds->findAndGetSequence(
            DcmTagKey(0x3006, 0x0010),
            refFrameSeq);

    if (!status.good() || !refFrameSeq)
    {
        qDebug() << "Cannot find Referenced Frame Sequence";
        return false;
    }

    for (unsigned long i = 0;
        i < refFrameSeq->card();
        i++)
    {
        DcmItem* item =
            refFrameSeq->getItem(i);

        if (!item)
            continue;

        OFString rtFOR;

        item->findAndGetOFString(
            DCM_FrameOfReferenceUID,
            rtFOR);

        if (rtFOR.empty())
            continue;

        qDebug()
            << "RTSTRUCT FOR:"
            << rtFOR.c_str();

        qDebug()
            << "CT FOR:"
            << volume->frameOfReferenceUID.c_str();

        if (rtFOR.c_str()
            == volume->frameOfReferenceUID)
        {
            return true;
        }
    }

    return false;
}

// =============================================
// PARSE STRUCTURES
// =============================================

bool RTStructLoader::parseStructures(
    DRTStructureSetIOD& rtStruct,
    StructureSetContainer& structureSet)
{
    auto& roiSeq =
        rtStruct.getStructureSetROISequence();

    for (unsigned long i = 0;
        i < roiSeq.getNumberOfItems();
        i++)
    {
        DRTStructureSetROISequence::Item&
            item = roiSeq.getItem(i);

        OFString roiName;

        item.getROIName(roiName);

        StructureModel structure;

        structure.name =
            roiName.c_str();

        structure.color =
            Qt::red;

        structure.visible =
            true;

        structureSet.addStructure(structure);
    }

    return true;
}

// =============================================
// PARSE CONTOURS
// =============================================

bool RTStructLoader::parseContours(
    DRTStructureSetIOD& rtStruct,
    StructureSetContainer& structureSet)
{
    auto& roiContourSeq =
        rtStruct.getROIContourSequence();

    for (unsigned long i = 0;
        i < roiContourSeq.getNumberOfItems();
        i++)
    {
        DRTROIContourSequence::Item&
            roiItem =
            roiContourSeq.getItem(i);

        Sint32 roiNumber = 0;

        roiItem.getReferencedROINumber(
            roiNumber);

        StructureModel* structure =
            structureSet.getStructureByIndex(
                roiNumber - 1);

        if (!structure)
            continue;

        // =============================
        // COLOR
        // =============================

        Sint32 r = 255;
        Sint32 g = 0;
        Sint32 b = 0;

        roiItem.getROIDisplayColor(r, 0);
        roiItem.getROIDisplayColor(g, 1);
        roiItem.getROIDisplayColor(b, 2);

        structure->color =
            QColor(r, g, b);

        // =============================
        // CONTOUR SEQUENCE
        // =============================

        auto& contourSeq =
            roiItem.getContourSequence();

        for (unsigned long j = 0;
            j < contourSeq.getNumberOfItems();
            j++)
        {
            DRTContourSequence::Item&
                contourItem =
                contourSeq.getItem(j);

            OFVector<Float64> data;

            contourItem.getContourData(data);

            Contour contour;

            for (size_t k = 0;
                k + 2 < data.size();
                k += 3)
            {
                RTPoint p;

                p.x = data[k];
                p.y = data[k + 1];
                p.z = data[k + 2];

                contour.points.push_back(p);
            }

            // =============================
            // SLICE Z
            // =============================

            if (!contour.points.empty())
            {
                contour.sliceZ =
                    contour.points[0].z;
            }

            structure->addContour(contour);
        }
    }

    return true;
}

bool RTStructLoader::parseROIObservations(
    DRTStructureSetIOD& rtStruct,
    StructureSetContainer& structureSet)
{
    auto& obsSeq =
        rtStruct.getRTROIObservationsSequence();

    for (unsigned long i = 0;
        i < obsSeq.getNumberOfItems();
        i++)
    {
        auto& item =
            obsSeq.getItem(i);

        Sint32 roiNumber = 0;

        item.getReferencedROINumber(
            roiNumber);

        StructureModel* structure =
            structureSet.getStructureByIndex(
                roiNumber - 1);

        if (!structure)
            continue;

        OFString roiType;

        item.getRTROIInterpretedType(
            roiType);

        structure->type =
            roiType.c_str();
    }

    return true;
}