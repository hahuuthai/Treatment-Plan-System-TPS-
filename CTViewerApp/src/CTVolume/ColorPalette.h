#pragma once
#include <QColor>
#include <QVector>

class StructureSetContainer;

// Sinh màu chưa được dùng trong structureSet (dùng chung cho Operations & Margin panel)
class ColorPalette
{
public:
    static QColor generateUniqueColor(const StructureSetContainer* structureSet);
};
