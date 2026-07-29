#include "DoseGridModel.h"

#include <algorithm>
#include <QDebug>

DoseGridModel::DoseGridModel()
{
    spacing[0] = 1.0;
    spacing[1] = 1.0;
    spacing[2] = 1.0;

    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;

    direction[0] = 1.0;
    direction[1] = 0.0;
    direction[2] = 0.0;

    direction[3] = 0.0;
    direction[4] = 1.0;
    direction[5] = 0.0;
}

bool DoseGridModel::isValid() const
{
    return
        width > 0 &&
        height > 0 &&
        depth > 0 &&
        dose.size() ==
        static_cast<size_t>(width * height * depth);
}

bool DoseGridModel::contains(
    int x,
    int y,
    int z) const
{
    return
        x >= 0 &&
        y >= 0 &&
        z >= 0 &&
        x < width &&
        y < height &&
        z < depth;
}

void DoseGridModel::clear()
{
    width = 0;
    height = 0;
    depth = 0;

    dose.clear();

    frameOffsets.clear();

    doseGridScaling = 1.0;

    spacing[0] = spacing[1] = spacing[2] = 1.0;

    origin[0] = origin[1] = origin[2] = 0.0;

    direction[0] = 1.0;
    direction[1] = 0.0;
    direction[2] = 0.0;
    direction[3] = 0.0;
    direction[4] = 1.0;
    direction[5] = 0.0;

    doseUnit.clear();
    doseType.clear();
    doseSummationType.clear();

    frameOfReferenceUID.clear();
    studyInstanceUID.clear();
    seriesInstanceUID.clear();
    sopInstanceUID.clear();
}

void DoseGridModel::resize(
    int w,
    int h,
    int d)
{
    if (w <= 0 || h <= 0 || d <= 0)
    {
        qDebug()
            << "DoseGridModel::resize: invalid dimensions"
            << w << h << d
            << "- clearing grid instead";

        width = 0;
        height = 0;
        depth = 0;

        dose.clear();

        return;
    }

    width = w;
    height = h;
    depth = d;

    dose.assign(
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        static_cast<size_t>(depth),
        0.0f);
}

int DoseGridModel::index(
    int x,
    int y,
    int z) const
{
    return
        z * width * height +
        y * width +
        x;
}

float&
DoseGridModel::at(
    int x,
    int y,
    int z)
{
    return
        dose[index(x, y, z)];
}

const float&
DoseGridModel::at(
    int x,
    int y,
    int z) const
{
    return
        dose[index(x, y, z)];
}

float DoseGridModel::getMaximumDose() const
{
    if (dose.empty())
        return 0.0f;

    return
        *std::max_element(
            dose.begin(),
            dose.end());
}

float DoseGridModel::getMinimumDose() const
{
    if (dose.empty())
        return 0.0f;

    return
        *std::min_element(
            dose.begin(),
            dose.end());
}