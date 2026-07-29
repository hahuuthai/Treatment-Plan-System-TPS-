#include "StructureVolumeCalculator.h"

double StructureVolumeCalculator::computeVolumeCM3(
    const MaskVolume& mask,
    const CTVolume& volume)
{
    long long count = 0;

    for (auto v : mask.voxels)
    {
        if (v)
            count++;
    }

    double voxelVolume =
        volume.spacing[0] *
        volume.spacing[1] *
        volume.spacing[2];

    double mm3 =
        voxelVolume * count;

    return mm3 / 1000.0;
}