#include "StructureMarginExpansion.h"

#include <cmath>

MaskVolume StructureMarginExpansion::expand(
    const MaskVolume& input,
    const CTVolume& volume,
    double marginMM)
{
    MaskVolume out;

    out.width = input.width;
    out.height = input.height;
    out.depth = input.depth;
    out.voxels = input.voxels;

    int rx = int(std::ceil(marginMM / volume.spacing[0]));
    int ry = int(std::ceil(marginMM / volume.spacing[1]));
    int rz = int(std::ceil(marginMM / volume.spacing[2]));

    struct Offset
    {
        int dx;
        int dy;
        int dz;
    };

    std::vector<Offset> kernel;

    double r2 = marginMM * marginMM;

    for (int dz = -rz; dz <= rz; ++dz)
    {
        for (int dy = -ry; dy <= ry; ++dy)
        {
            for (int dx = -rx; dx <= rx; ++dx)
            {
                double d2 =
                    dx * dx * volume.spacing[0] * volume.spacing[0] +
                    dy * dy * volume.spacing[1] * volume.spacing[1] +
                    dz * dz * volume.spacing[2] * volume.spacing[2];

                if (d2 <= r2)
                    kernel.push_back({ dx, dy, dz });
            }
        }
    }

    for (int z = 0; z < input.depth; ++z)
    {
        for (int y = 0; y < input.height; ++y)
        {
            for (int x = 0; x < input.width; ++x)
            {
                if (!input.at(x, y, z))
                    continue;

                for (const auto& k : kernel)
                {
                    int nx = x + k.dx;
                    int ny = y + k.dy;
                    int nz = z + k.dz;

                    if (nx < 0 || ny < 0 || nz < 0)
                        continue;

                    if (nx >= input.width ||
                        ny >= input.height ||
                        nz >= input.depth)
                        continue;

                    out.at(nx, ny, nz) = 1;
                }
            }
        }
    }

    return out;
}