#include "CoordinateTransformer.h"

#include <cmath>
#include "CTVolume/CTVolume.h"

// =============================================
// PHYSICAL -> VOXEL
// =============================================

bool CoordinateTransformer::physicalToVoxel(
    const CTVolume& volume,
    double x,
    double y,
    double z,
    int& i,
    int& j,
    int& k)
{
    // =========================================
    // SIMPLE AXIAL MAPPING
    // =========================================

    double dx =
        x - volume.origin[0];

    double dy =
        y - volume.origin[1];

    double dz =
        z - volume.origin[2];

    i = static_cast<int>(
        std::round(dx / volume.spacing[0]));

    j = static_cast<int>(
        std::round(dy / volume.spacing[1]));

    k = static_cast<int>(
        std::round(dz / volume.spacing[2]));

    // =========================================
    // BOUND CHECK
    // =========================================

    if (i < 0 || i >= volume.width)
        return false;

    if (j < 0 || j >= volume.height)
        return false;

    if (k < 0 || k >= volume.depth)
        return false;

    return true;
}

// =============================================
// CONTOUR -> SLICE INDEX
// =============================================

int CoordinateTransformer::contourToSliceIndex(
    const Contour& contour,
    const CTVolume& volume)
{
    if (contour.points.empty())
        return -1;

    // =========================================
    // USE FIRST POINT Z
    // =========================================

    double z =
        contour.points[0].z;

    double dz =
        z - volume.origin[2];

    int slice =
        static_cast<int>(
            std::round(
                dz / volume.spacing[2]));

    if (slice < 0 ||
        slice >= volume.depth)
    {
        return -1;
    }

    return slice;
}

// =============================================
// CHECK CONTOUR ON SLICE
// =============================================

bool CoordinateTransformer::isContourOnSlice(
    const Contour& contour,
    const CTVolume& volume,
    int sliceIndex,
    double tolerance)
{
    if (contour.points.empty())
        return false;

    double contourZ =
        contour.points[0].z;

    double sliceZ =
        volume.origin[2] +
        sliceIndex * volume.spacing[2];

    return std::abs(
        contourZ - sliceZ) <= tolerance;
}