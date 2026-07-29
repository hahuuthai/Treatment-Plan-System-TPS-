#pragma once

#include <string>

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmrt/drtstrct.h>

#include "CTVolume/CTVolume.h"
#include "StructureSetContainer.h"

// =============================================
// RTSTRUCT LOADER
// =============================================

class RTStructLoader
{
public:

    // =========================================
    // LOAD RTSTRUCT
    // =========================================

    static bool load(
        const std::string& filePath,
        const CTVolume* volume,
        StructureSetContainer& structureSet);

private:

    // =========================================
    // VALIDATE FRAME OF REFERENCE UID
    // =========================================

    static bool validateFrameOfReference(
        DcmDataset* ds,
        const CTVolume* volume);

    // =========================================
    // PARSE ROI STRUCTURES
    // =========================================

    static bool parseStructures(
        DRTStructureSetIOD& rtStruct,
        StructureSetContainer& structureSet);

    // =========================================
    // PARSE ROI CONTOURS
    // =========================================

    static bool parseContours(
        DRTStructureSetIOD& rtStruct,
        StructureSetContainer& structureSet);
	
    // =========================================
    // PARSE ROI OBSERVATIONS
    // =========================================
    static bool parseROIObservations(
        DRTStructureSetIOD& rtStruct,
        StructureSetContainer& structureSet);
};