#pragma once

#include "CTVolume/CTVolume.h"
#include "StructureModel.h"

class CoordinateTransformer
{
public:

    // =========================================
    // PHYSICAL (mm) -> VOXEL INDEX
    // =========================================

    static bool physicalToVoxel(
        const CTVolume& volume,
        double x,
        double y,
        double z,
        int& i,
        int& j,
        int& k);

    // =========================================
    // CHECK CONTOUR BELONGS TO SLICE
    // =========================================

    static bool isContourOnSlice(
        const Contour& contour,
        const CTVolume& volume,
        int sliceIndex,
        double tolerance = 0.5);

    // =========================================
    // GET CONTOUR SLICE INDEX
    // =========================================

    static int contourToSliceIndex(
        const Contour& contour,
        const CTVolume& volume);
};