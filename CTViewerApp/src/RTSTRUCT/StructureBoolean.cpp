#include "StructureBoolean.h"
#include <QDebug>

MaskVolume StructureBoolean::Union(
    const MaskVolume& A,
    const MaskVolume& B)
{
    if (A.voxels.size() != B.voxels.size())
    {
        qDebug()
            << "Union size mismatch";

        return MaskVolume();
    }

    MaskVolume out;

    out.width = A.width;
    out.height = A.height;
    out.depth = A.depth;

    out.voxels.resize(A.voxels.size());

    for (size_t i = 0; i < A.voxels.size(); ++i)
    {
        out.voxels[i] =
            A.voxels[i] || B.voxels[i];
    }

    return out;
}

MaskVolume StructureBoolean::Intersect(
    const MaskVolume& A,
    const MaskVolume& B)
{
    if (A.voxels.size() != B.voxels.size())
        return MaskVolume();

    MaskVolume out;

    out.width = A.width;
    out.height = A.height;
    out.depth = A.depth;

    out.voxels.resize(A.voxels.size());

    for (size_t i = 0; i < A.voxels.size(); ++i)
    {
        out.voxels[i] =
            A.voxels[i] && B.voxels[i];
    }

    return out;
}

MaskVolume StructureBoolean::Subtract(
    const MaskVolume& A,
    const MaskVolume& B)
{
    if (A.voxels.size() != B.voxels.size())
        return MaskVolume();

    MaskVolume out;

    out.width = A.width;
    out.height = A.height;
    out.depth = A.depth;

    out.voxels.resize(A.voxels.size());

    for (size_t i = 0; i < A.voxels.size(); ++i)
    {
        out.voxels[i] =
            A.voxels[i] && !B.voxels[i];
    }

    return out;
}