#pragma once

#include <unordered_map>

#include "CoordinateAlignment.h"
#include "RTDOSE/DoseGridModel.h"
#include "CTVolume/CTVolume.h"
#include "RTSTRUCT/MaskVolume.h"

class DoseSampler
{
public:

    enum class SamplingMode
    {
        Nearest,
        Trilinear
    };

    //-------------------------------------------------
    // Core Dose Sampling (physical dose-grid index space)
    //-------------------------------------------------

    static float sampleDose(
        const DoseGridModel& dose,
        double x,
        double y,
        double z,
        SamplingMode mode =
        SamplingMode::Trilinear);

    //-------------------------------------------------
    // Display Track
    //
    // Fast path for interactive rendering (slice viewer,
    // dose color-wash, etc). Cached, defaults to Nearest.
    // Precision here is intentionally traded for speed -
    // acceptable since it's re-evaluated every frame and
    // sub-voxel error isn't visible at display resolution.
    //-------------------------------------------------

    static float sampleDisplay(
        const CTVolume& ct,
        const DoseGridModel& dose,
        int x,
        int y,
        int z,
        SamplingMode mode =
        SamplingMode::Nearest);

    // Deprecated alias kept for existing call sites.
    // New code should call sampleDisplay() directly.
    static float sampleCT(
        const CTVolume& ct,
        const DoseGridModel& dose,
        int x,
        int y,
        int z,
        SamplingMode mode =
        SamplingMode::Trilinear);

    //-------------------------------------------------
    // Quantitative Track
    //
    // High-precision path for clinical metrics (mean/max/min
    // dose, DVH). Always trilinear, never cached, and
    // optionally supersamples each CT voxel against the dose
    // grid to reduce partial-volume error at structure
    // boundaries where CT voxels are much finer than dose
    // voxels.
    //-------------------------------------------------

    static float sampleQuantitative(
        const CTVolume& ct,
        const DoseGridModel& dose,
        int x,
        int y,
        int z,
        int subSamples = 2);

    // Call whenever the active DoseGridModel changes (new plan
    // loaded, plan reloaded, etc). The display cache is keyed by
    // CT voxel + mode only, not by grid identity, so it must be
    // invalidated manually on grid swap.
    static void clearCache();

    static float meanDose(
        const MaskVolume& mask,
        const CTVolume& ct,
        const DoseGridModel& dose);

    static float maximumDose(
        const MaskVolume& mask,
        const CTVolume& ct,
        const DoseGridModel& dose);

    static float minimumDose(
        const MaskVolume& mask,
        const CTVolume& ct,
        const DoseGridModel& dose);

private:

    //-------------------------------------------------
    // Internal
    //-------------------------------------------------

    static float nearest(
        const DoseGridModel& dose,
        double x,
        double y,
        double z);

    static float trilinear(
        const DoseGridModel& dose,
        double x,
        double y,
        double z);

    //-------------------------------------------------
    // Display Cache
    //
    // Keyed by CT voxel AND sampling mode - the two tracks
    // must never share a cache slot, since a Nearest sample
    // and a Trilinear sample at the same voxel are different
    // values.
    //-------------------------------------------------

    struct CacheKey
    {
        int x;
        int y;
        int z;
        SamplingMode mode;

        bool operator==(
            const CacheKey& other) const
        {
            return
                x == other.x &&
                y == other.y &&
                z == other.z &&
                mode == other.mode;
        }
    };

    struct CacheHash
    {
        size_t operator()(
            const CacheKey& k) const
        {
            // Cast to an unsigned type before multiplying: the
            // previous int*int multiplication is signed-integer
            // overflow, which is undefined behavior in C++ for
            // voxel coordinates large enough to overflow int.
            return
                (static_cast<size_t>(k.x) * 73856093u) ^
                (static_cast<size_t>(k.y) * 19349663u) ^
                (static_cast<size_t>(k.z) * 83492791u) ^
                (static_cast<size_t>(k.mode) * 2654435761u);
        }
    };

    static std::unordered_map<
        CacheKey,
        float,
        CacheHash> displayCache;
};