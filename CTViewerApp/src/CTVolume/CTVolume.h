#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <string>

class CTVolume
{
public:
    std::vector<int16_t> voxels;

    int width = 0;
    int height = 0;
    int depth = 0;

    double spacing[3] = { 1.0, 1.0, 1.0 };
    double origin[3] = { 0, 0, 0 };          // Image Position (Patient)
    double direction[6] = { 0 };             // Image Orientation (Patient)
    double rescaleSlope = 1.0;
    double rescaleIntercept = 0.0;

    std::vector<std::array<double, 3>> slicePositions;
    std::string modality;
    std::string frameOfReferenceUID;

    void clear()
    {
        voxels.clear();
        width = height = depth = 0;
    }
};