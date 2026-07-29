#pragma once

#include <vector>
#include <cstdint>

struct MaskVolume
{
    int width = 0;
    int height = 0;
    int depth = 0;

    std::vector<uint8_t> voxels;

    inline uint8_t& at(
        int x,
        int y,
        int z)
    {
        return voxels[
            z * width * height +
                y * width +
                x];
    }

    inline const uint8_t& at(
        int x,
        int y,
        int z) const
    {
        return voxels[
            z * width * height +
                y * width +
                x];
    }
};