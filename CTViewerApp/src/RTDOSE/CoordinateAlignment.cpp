#include "CoordinateAlignment.h"

#include <cmath>
#include <algorithm>

namespace
{
    //-------------------------------------------------
    // Cross Product
    //-------------------------------------------------

    std::array<double, 3> cross(
        const std::array<double, 3>& a,
        const std::array<double, 3>& b)
    {
        return
        {
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0]
        };
    }

    //-------------------------------------------------

    struct Matrix3
    {
        double m[3][3]{};
    };

    //-------------------------------------------------

    Matrix3 buildMatrix(
        const double direction[6],
        const double spacing[3])
    {
        std::array<double, 3> row =
        {
            direction[0],
            direction[1],
            direction[2]
        };

        std::array<double, 3> col =
        {
            direction[3],
            direction[4],
            direction[5]
        };

        auto slice =
            cross(row, col);

        Matrix3 mat;

        // DICOM PixelSpacing = [row spacing, column spacing].
        // Row spacing is the distance covered per unit increment
        // of the ROW index, i.e. along the column-direction
        // cosine; column spacing is the distance covered per unit
        // increment of the COLUMN index, i.e. along the
        // row-direction cosine. "row"/"col" here are the IOP
        // direction cosines themselves (row = direction[0:3] =
        // row-direction cosine, col = direction[3:6] =
        // column-direction cosine), so they pair with spacing[1]
        // and spacing[0] respectively - not spacing[0]/spacing[1].
        mat.m[0][0] = row[0] * spacing[1];
        mat.m[1][0] = row[1] * spacing[1];
        mat.m[2][0] = row[2] * spacing[1];

        mat.m[0][1] = col[0] * spacing[0];
        mat.m[1][1] = col[1] * spacing[0];
        mat.m[2][1] = col[2] * spacing[0];

        mat.m[0][2] = slice[0];
        mat.m[1][2] = slice[1];
        mat.m[2][2] = slice[2];

        return mat;
    }

    //-------------------------------------------------

    Matrix3 inverse(
        const Matrix3& a)
    {
        Matrix3 inv;

        double det =
            a.m[0][0] *
            (
                a.m[1][1] * a.m[2][2] -
                a.m[2][1] * a.m[1][2]
                )
            -
            a.m[0][1] *
            (
                a.m[1][0] * a.m[2][2] -
                a.m[2][0] * a.m[1][2]
                )
            +
            a.m[0][2] *
            (
                a.m[1][0] * a.m[2][1] -
                a.m[2][0] * a.m[1][1]
                );

        if (std::abs(det) < 1e-12)
            return inv;

        double id = 1.0 / det;

        inv.m[0][0] = (a.m[1][1] * a.m[2][2] - a.m[2][1] * a.m[1][2]) * id;
        inv.m[0][1] = (a.m[0][2] * a.m[2][1] - a.m[0][1] * a.m[2][2]) * id;
        inv.m[0][2] = (a.m[0][1] * a.m[1][2] - a.m[0][2] * a.m[1][1]) * id;

        inv.m[1][0] = (a.m[1][2] * a.m[2][0] - a.m[1][0] * a.m[2][2]) * id;
        inv.m[1][1] = (a.m[0][0] * a.m[2][2] - a.m[0][2] * a.m[2][0]) * id;
        inv.m[1][2] = (a.m[1][0] * a.m[0][2] - a.m[0][0] * a.m[1][2]) * id;

        inv.m[2][0] = (a.m[1][0] * a.m[2][1] - a.m[2][0] * a.m[1][1]) * id;
        inv.m[2][1] = (a.m[2][0] * a.m[0][1] - a.m[0][0] * a.m[2][1]) * id;
        inv.m[2][2] = (a.m[0][0] * a.m[1][1] - a.m[1][0] * a.m[0][1]) * id;

        return inv;
    }

}

std::array<double, 3>
CoordinateAlignment::ctIndexToPatient(
    const CTVolume& ct,
    double i,
    double j,
    double k)
{
    Matrix3 m =
        buildMatrix(
            ct.direction,
            ct.spacing);

    auto slice =
        computeSliceDirection(
            ct.direction);

    return
    {
        ct.origin[0]
        + m.m[0][0] * i
        + m.m[0][1] * j
        + slice[0] * ct.spacing[2] * k,

        ct.origin[1]
        + m.m[1][0] * i
        + m.m[1][1] * j
        + slice[1] * ct.spacing[2] * k,

        ct.origin[2]
        + m.m[2][0] * i
        + m.m[2][1] * j
        + slice[2] * ct.spacing[2] * k
    };
}

std::array<double, 3>
CoordinateAlignment::patientToCTIndex(
    const CTVolume& ct,
    double x,
    double y,
    double z)
{
    Matrix3 m =
        buildMatrix(
            ct.direction,
            ct.spacing);

    Matrix3 inv =
        inverse(m);

    double px =
        x - ct.origin[0];

    double py =
        y - ct.origin[1];

    double pz =
        z - ct.origin[2];

    auto slice =
        computeSliceDirection(
            ct.direction);

    double k =
        (px * slice[0]
            + py * slice[1]
            + pz * slice[2])
        / ct.spacing[2];

    return
    {
        inv.m[0][0] * px +
        inv.m[0][1] * py,

        inv.m[1][0] * px +
        inv.m[1][1] * py,

        k
    };
}

std::array<double, 3>
CoordinateAlignment::doseIndexToPatient(
    const DoseGridModel& dose,
    double i,
    double j,
    double k)
{
    Matrix3 m =
        buildMatrix(
            dose.direction,
            dose.spacing);

    auto slice =
        computeSliceDirection(
            dose.direction);

    double offset = 0.0;

    if (!dose.frameOffsets.empty())
    {
        int k0 = (int)std::floor(k);
        int k1 = std::min(
            k0 + 1,
            (int)dose.frameOffsets.size() - 1);

        double t = k - k0;

        offset =
            (1.0 - t) * dose.frameOffsets[k0]
            + t * dose.frameOffsets[k1];
    }
    else
    {
        offset =
            k * dose.spacing[2];
    }

    return
    {
        dose.origin[0]
        + m.m[0][0] * i
        + m.m[0][1] * j
        + slice[0] * offset,

        dose.origin[1]
        + m.m[1][0] * i
        + m.m[1][1] * j
        + slice[1] * offset,

        dose.origin[2]
        + m.m[2][0] * i
        + m.m[2][1] * j
        + slice[2] * offset
    };
}

std::array<double, 3>
CoordinateAlignment::patientToDoseIndex(
    const DoseGridModel& dose,
    double x,
    double y,
    double z)
{
    Matrix3 m =
        buildMatrix(
            dose.direction,
            dose.spacing);

    Matrix3 inv =
        inverse(m);

    double px =
        x - dose.origin[0];

    double py =
        y - dose.origin[1];

    double pz =
        z - dose.origin[2];

    auto slice =
        computeSliceDirection(
            dose.direction);

    //-------------------------------------------------
    // X,Y
    //-------------------------------------------------

    double i =
        inv.m[0][0] * px +
        inv.m[0][1] * py;

    double j =
        inv.m[1][0] * px +
        inv.m[1][1] * py;

    //-------------------------------------------------
    // Z
    //-------------------------------------------------

    double distance =
        px * slice[0] +
        py * slice[1] +
        pz * slice[2];

    double k = 0.0;

    if (!dose.frameOffsets.empty())
    {
        auto it =
            std::lower_bound(
                dose.frameOffsets.begin(),
                dose.frameOffsets.end(),
                distance);

        if (it == dose.frameOffsets.begin())
        {
            k = 0.0;
        }
        else if (it == dose.frameOffsets.end())
        {
            k =
                double(dose.frameOffsets.size() - 1);
        }
        else
        {
            int upper =
                int(it - dose.frameOffsets.begin());

            int lower =
                upper - 1;

            double d0 =
                dose.frameOffsets[lower];

            double d1 =
                dose.frameOffsets[upper];

            double t =
                (distance - d0) /
                (d1 - d0);

            k =
                lower + t;
        }
    }
    else
    {
        k =
            distance /
            dose.spacing[2];
    }

    return
    {
        i,
        j,
        k
    };
}

std::array<double, 3>
CoordinateAlignment::ctIndexToDoseIndex(
    const CTVolume& ct,
    const DoseGridModel& dose,
    double i,
    double j,
    double k)
{
    auto patient =
        ctIndexToPatient(
            ct,
            i,
            j,
            k);

    return
        patientToDoseIndex(
            dose,
            patient[0],
            patient[1],
            patient[2]);
}

std::array<double, 3>
CoordinateAlignment::doseIndexToCTIndex(
    const DoseGridModel& dose,
    const CTVolume& ct,
    double i,
    double j,
    double k)
{
    auto patient =
        doseIndexToPatient(
            dose,
            i,
            j,
            k);

    return
        patientToCTIndex(
            ct,
            patient[0],
            patient[1],
            patient[2]);
}

std::array<double, 3>
CoordinateAlignment::computeSliceDirection(
    const double direction[6])
{
    std::array<double, 3> row =
    {
        direction[0],
        direction[1],
        direction[2]
    };

    std::array<double, 3> col =
    {
        direction[3],
        direction[4],
        direction[5]
    };

    return
    {
        row[1] * col[2] - row[2] * col[1],
        row[2] * col[0] - row[0] * col[2],
        row[0] * col[1] - row[1] * col[0]
    };
}