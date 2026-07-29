// =============================================
// StructureModel.cpp
// =============================================
#include "StructureModel.h"

// =============================================
// CONSTRUCTOR
// =============================================

StructureModel::StructureModel(
    const std::string& structureName)
{
    name = structureName;
}

// =============================================
// ADD CONTOUR
// =============================================

void StructureModel::addContour(
    const Contour& contour)
{
    if (contour.points.size() < 3)
        return;

    contours.push_back(contour);
}

// =============================================
// REMOVE CONTOUR
// =============================================

void StructureModel::removeContour(
    size_t index)
{
    if (index >= contours.size())
        return;

    contours.erase(
        contours.begin() + index);
}

// =============================================
// CLEAR CONTOURS
// =============================================

void StructureModel::clearContours()
{
    contours.clear();
}

// =============================================
// CONTOUR COUNT
// =============================================

size_t StructureModel::contourCount() const
{
    return contours.size();
}

// =============================================
// HAS CONTOURS
// =============================================

bool StructureModel::hasContours() const
{
    return !contours.empty();
}

// =============================================
// CREATE MASK
// =============================================

void StructureModel::createMask(
    int width,
    int height,
    int depth)
{
    mask.width = width;
    mask.height = height;
    mask.depth = depth;

    mask.voxels.assign(
        width * height * depth,
        0);
}

// =============================================
// CLEAR MASK
// =============================================

void StructureModel::clearMask()
{
    mask.voxels.clear();

    mask.width = 0;
    mask.height = 0;
    mask.depth = 0;
}

// =============================================
// HAS MASK
// =============================================

bool StructureModel::hasMask() const
{
    return !mask.voxels.empty();
}

// =============================================
// SET COLOR
// =============================================

void StructureModel::setColor(
    const QColor& c)
{
    color = c;
}

// =============================================
// GET COLOR
// =============================================

QColor StructureModel::getColor() const
{
    return color;
}

// =============================================
// SET VISIBLE
// =============================================

void StructureModel::setVisible(
    bool state)
{
    visible = state;
}

// =============================================
// IS VISIBLE
// =============================================

bool StructureModel::isVisible() const
{
    return visible;
}

// =============================================
// CLEAR
// =============================================

void StructureModel::clear()
{
    clearContours();

    clearMask();
}