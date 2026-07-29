#pragma once

#include "MaskVolume.h"
#include "StructureModel.h"
#include "CTVolume/CTVolume.h"

class ContourVoxelizer
{
public:

    static MaskVolume createMask(
        const StructureModel& structure,
        const CTVolume& volume);
};