#pragma once

#include <array>

#include "CTVolume/CTVolume.h"
#include "RTDOSE/DoseGridModel.h"

class CoordinateAlignment
{
public:

    //---------------------------------------
    // CT <-> Patient
    //---------------------------------------

    static std::array<double, 3>
        ctIndexToPatient(
            const CTVolume& ct,
            double i,
            double j,
            double k);

    static std::array<double, 3>
        patientToCTIndex(
            const CTVolume& ct,
            double x,
            double y,
            double z);

    //---------------------------------------
    // Dose <-> Patient
    //---------------------------------------

    static std::array<double, 3>
        doseIndexToPatient(
            const DoseGridModel& dose,
            double i,
            double j,
            double k);

    static std::array<double, 3>
        patientToDoseIndex(
            const DoseGridModel& dose,
            double x,
            double y,
            double z);

    //---------------------------------------
    // CT <-> Dose
    //---------------------------------------

    static std::array<double, 3>
        ctIndexToDoseIndex(
            const CTVolume& ct,
            const DoseGridModel& dose,
            double i,
            double j,
            double k);

    static std::array<double, 3>
        doseIndexToCTIndex(
            const DoseGridModel& dose,
            const CTVolume& ct,
            double i,
            double j,
            double k);

private:

    //---------------------------------------
    // Slice direction = Row x Column
    //---------------------------------------

    static std::array<double, 3>
        computeSliceDirection(
            const double direction[6]);
};