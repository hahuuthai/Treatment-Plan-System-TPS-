#include "DoseSampler.h"

#include <cmath>
#include <algorithm>
#include <limits>
#include <QDebug>

std::unordered_map<
    DoseSampler::CacheKey,
    float,
    DoseSampler::CacheHash>
    DoseSampler::displayCache;

// ================================
// Core Sampling
// ================================

float DoseSampler::sampleDose(
    const DoseGridModel& dose,
    double x,
    double y,
    double z,
    SamplingMode mode)
{
    if (mode == SamplingMode::Nearest)
    {
        return nearest(
            dose,
            x,
            y,
            z);
    }

    return trilinear(
        dose,
        x,
        y,
        z);
}

// ===============================
// Nearest
// ===============================

float DoseSampler::nearest(
    const DoseGridModel& dose,
    double x,
    double y,
    double z)
{
    int ix =
        int(std::round(x));

    int iy =
        int(std::round(y));

    int iz =
        int(std::round(z));

    if (!dose.contains(
        ix,
        iy,
        iz))
    {
        return 0.0f;
    }

    return dose.at(
        ix,
        iy,
        iz);
}

// ===============================
// Trilinear
// ===============================

float DoseSampler::trilinear(
    const DoseGridModel& dose,
    double x,
    double y,
    double z)
{
    int x0 =
        int(std::floor(x));

    int y0 =
        int(std::floor(y));

    int z0 =
        int(std::floor(z));

    if (!dose.contains(x0, y0, z0))
    {
        return 0.0f;
    }

    // Clamp the "+1" neighbor to the last valid index instead of
    // stepping outside the grid. When x/y/z lands exactly on the
    // last slice/row/column (dx/dy/dz == 0), the neighbor and base
    // corner are the same voxel, so clamping doesn't change the
    // interpolated result - it just avoids an out-of-range index.
    int x1 =
        std::min(x0 + 1, dose.width - 1);

    int y1 =
        std::min(y0 + 1, dose.height - 1);

    int z1 =
        std::min(z0 + 1, dose.depth - 1);

    double dx =
        x - x0;

    double dy =
        y - y0;

    double dz =
        z - z0;

    float c000 = dose.at(x0, y0, z0);
    float c100 = dose.at(x1, y0, z0);
    float c010 = dose.at(x0, y1, z0);
    float c110 = dose.at(x1, y1, z0);
    float c001 = dose.at(x0, y0, z1);
    float c101 = dose.at(x1, y0, z1);
    float c011 = dose.at(x0, y1, z1);
    float c111 = dose.at(x1, y1, z1);

    float c00 =
        c000 * (1.0 - dx)
        + c100 * dx;

    float c10 =
        c010 * (1.0 - dx)
        + c110 * dx;

    float c01 =
        c001 * (1.0 - dx)
        + c101 * dx;

    float c11 =
        c011 * (1.0 - dx)
        + c111 * dx;

    float c0 =
        c00 * (1.0 - dy)
        + c10 * dy;

    float c1 =
        c01 * (1.0 - dy)
        + c11 * dy;

    return
        c0 * (1.0 - dz)
        + c1 * dz;
}

// ================================
// Display Track
// ================================

float DoseSampler::sampleDisplay(
    const CTVolume& ct,
    const DoseGridModel& dose,
    int x,
    int y,
    int z,
    SamplingMode mode)
{
    //-------------------------------------------------
    // Cache (keyed on voxel + mode)
    //-------------------------------------------------

    CacheKey key
    {
        x,
        y,
        z,
        mode
    };

    auto it =
        displayCache.find(key);

    if (it != displayCache.end())
    {
        return it->second;
    }

    //-------------------------------------------------
    // CT index -> Dose index
    //-------------------------------------------------

    auto dosePos =
        CoordinateAlignment::ctIndexToDoseIndex(
            ct,
            dose,
            x,
            y,
            z);

    float value =
        sampleDose(
            dose,
            dosePos[0],
            dosePos[1],
            dosePos[2],
            mode);

    //-------------------------------------------------
    // cache
    //-------------------------------------------------

    if (displayCache.size() > 50000)
    {
        displayCache.clear();
    }

    displayCache[key] =
        value;

    return value;
}

// Deprecated alias - forwards to the Display track so existing
// call sites keep working while callers migrate to sampleDisplay().
float DoseSampler::sampleCT(
    const CTVolume& ct,
    const DoseGridModel& dose,
    int x,
    int y,
    int z,
    SamplingMode mode)
{
    return sampleDisplay(
        ct,
        dose,
        x,
        y,
        z,
        mode);
}

void DoseSampler::clearCache()
{
    displayCache.clear();
}

// ================================
// Quantitative Track
// ================================
//
// Always trilinear, never cached. Optionally supersamples each
// CT voxel with a subSamples^3 sub-grid so that voxels near a
// structure boundary - where dose-grid voxels (here 3mm) are
// coarser than CT voxels (here ~1mm) - aren't reduced to a
// single point sample. subSamples = 1 reproduces a plain
// single-point trilinear sample at the voxel center.

float DoseSampler::sampleQuantitative(
    const CTVolume& ct,
    const DoseGridModel& dose,
    int x,
    int y,
    int z,
    int subSamples)
{
    if (subSamples < 1)
    {
        subSamples = 1;
    }

    double sum = 0.0;
    int count = 0;

    for (int sz = 0; sz < subSamples; sz++)
    {
        for (int sy = 0; sy < subSamples; sy++)
        {
            for (int sx = 0; sx < subSamples; sx++)
            {
                double fx =
                    x - 0.5 + (sx + 0.5) / subSamples;

                double fy =
                    y - 0.5 + (sy + 0.5) / subSamples;

                double fz =
                    z - 0.5 + (sz + 0.5) / subSamples;

                auto dosePos =
                    CoordinateAlignment::ctIndexToDoseIndex(
                        ct,
                        dose,
                        fx,
                        fy,
                        fz);

                sum +=
                    trilinear(
                        dose,
                        dosePos[0],
                        dosePos[1],
                        dosePos[2]);

                count++;
            }
        }
    }

    if (count == 0)
        return 0.0f;

    return
        static_cast<float>(
            sum / count);
}

// ================================
// Mean Dose
// ================================

float DoseSampler::meanDose(
    const MaskVolume& mask,
    const CTVolume& ct,
    const DoseGridModel& dose)
{
    double sum = 0.0;
    int count = 0;

    for (int z = 0; z < mask.depth; z++)
    {
        for (int y = 0; y < mask.height; y++)
        {
            for (int x = 0; x < mask.width; x++)
            {
                if (!mask.at(x, y, z))
                    continue;

                sum +=
                    sampleQuantitative(
                        ct,
                        dose,
                        x,
                        y,
                        z);

                count++;
            }
        }
    }

    if (count == 0)
        return 0.0f;

    return
        static_cast<float>(
            sum / count);
}


// =========================================
// Maximum Dose
// =========================================


float DoseSampler::maximumDose(
    const MaskVolume& mask,
    const CTVolume& ct,
    const DoseGridModel& dose)
{
    float maxDose =
        -std::numeric_limits<float>::max();

    bool found = false;

    for (int z = 0; z < mask.depth; z++)
    {
        for (int y = 0; y < mask.height; y++)
        {
            for (int x = 0; x < mask.width; x++)
            {
                if (!mask.at(x, y, z))
                    continue;

                float d =
                    sampleQuantitative(
                        ct,
                        dose,
                        x,
                        y,
                        z);

                maxDose =
                    std::max(
                        maxDose,
                        d);

                found = true;
            }
        }
    }

    if (!found)
        return 0.0f;

    return maxDose;
}


// =========================================
// Minimum Dose
// =========================================


float DoseSampler::minimumDose(
    const MaskVolume& mask,
    const CTVolume& ct,
    const DoseGridModel& dose)
{
    float minDose =
        std::numeric_limits<float>::max();

    bool found = false;

    for (int z = 0; z < mask.depth; z++)
    {
        for (int y = 0; y < mask.height; y++)
        {
            for (int x = 0; x < mask.width; x++)
            {
                if (!mask.at(x, y, z))
                    continue;

                float d =
                    sampleQuantitative(
                        ct,
                        dose,
                        x,
                        y,
                        z);

                minDose =
                    std::min(
                        minDose,
                        d);

                found = true;
            }
        }
    }

    if (!found)
        return 0.0f;

    return minDose;
}