// =============================================
// StructureModel.h
// =============================================
#pragma once

#include <vector>
#include <string>
#include "MaskVolume.h"
#include <QColor>

// =============================================
// RT POINT
// =============================================
struct RTPoint
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// =============================================
// CONTOUR
// =============================================
struct Contour
{
    std::vector<RTPoint> points;

    double sliceZ = 0.0;

    bool empty() const
    {
        return points.empty();
    }

    size_t size() const
    {
        return points.size();
    }
};

// =============================================
// STRUCTURE MODEL
// =============================================
class StructureModel
{
public:

    std::string name;

    std::string type;

    QColor color = Qt::red;

    bool visible = true;

    MaskVolume mask;

    double volumeCM3 = 0.0;

    // =========================================
    // RAW CONTOUR DATA
    // =========================================

    std::vector<Contour> contours;


public:

    StructureModel() = default;

    explicit StructureModel(
        const std::string& structureName);

    // =========================================
    // CONTOUR
    // =========================================

    void addContour(
        const Contour& contour);

    void removeContour(
        size_t index);

    void clearContours();

    size_t contourCount() const;

    bool hasContours() const;

    // =========================================
    // MASK
    // =========================================

    void createMask(
        int width,
        int height,
        int depth);

    void clearMask();

    bool hasMask() const;

    // =========================================
    // COLOR
    // =========================================

    void setColor(
        const QColor& c);

    QColor getColor() const;

    // =========================================
    // VISIBILITY
    // =========================================

    void setVisible(
        bool state);

    bool isVisible() const;

    // =========================================
    // CLEAR
    // =========================================

    void clear();
};