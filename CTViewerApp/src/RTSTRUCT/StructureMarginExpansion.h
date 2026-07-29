#pragma once

#include "MaskVolume.h"
#include "CTVolume/CTVolume.h"

class StructureMarginExpansion
{
public:

    static MaskVolume expand(
        const MaskVolume& input,
        const CTVolume& volume,
        double marginMM);
};