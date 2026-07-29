#pragma once

#include "MaskVolume.h"

class StructureBoolean
{
public:

    static MaskVolume Union(
        const MaskVolume& A,
        const MaskVolume& B);

    static MaskVolume Intersect(
        const MaskVolume& A,
        const MaskVolume& B);

    static MaskVolume Subtract(
        const MaskVolume& A,
        const MaskVolume& B);
};