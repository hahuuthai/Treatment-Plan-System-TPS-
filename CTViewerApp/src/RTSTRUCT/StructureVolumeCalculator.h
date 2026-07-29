#pragma once

#include "MaskVolume.h"
#include "CTVolume/CTVolume.h"

class StructureVolumeCalculator
{
public:

    static double computeVolumeCM3(
        const MaskVolume& mask,
        const CTVolume& volume);
};