#pragma once
#include <QImage>
#include <QString>
#include <array>
#include "CTVolume.h"

class StructureSetContainer;

// Thuần render: không đụng tới QWidget/QMainWindow, có thể unit-test độc lập.
class SliceRenderer
{
public:
    // Vẽ 1 slice CT (grayscale theo minLimit/maxLimit) + overlay contour/mask của structureSet
    static QImage renderSlice(
        const CTVolume& volume,
        int z,
        double minLimit,
        double maxLimit,
        const StructureSetContainer* structureSet);

    // Nhãn hướng (L/R/A/P/H/F) dựa trên vector hướng ảnh
    static QString getOrientationLabel(const std::array<double, 3>& v);
};
