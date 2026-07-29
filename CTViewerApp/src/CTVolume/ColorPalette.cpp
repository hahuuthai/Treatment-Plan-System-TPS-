#include "ColorPalette.h"
#include "RTSTRUCT/StructureSetContainer.h"

QColor ColorPalette::generateUniqueColor(const StructureSetContainer* structureSet)
{
    static const QVector<QColor> palette =
    {
        QColor(255,   0,   0),
        QColor(0, 255,   0),
        QColor(0,   0, 255),
        QColor(255, 255,   0),
        QColor(255,   0, 255),
        QColor(0, 255, 255),
        QColor(255, 128,   0),
        QColor(128,   0, 255),
        QColor(255, 128, 128),
        QColor(128, 255, 128),
        QColor(128, 128, 255),
        QColor(255, 200,   0),
        QColor(255, 100, 200),
        QColor(100, 255, 200),
        QColor(200, 255, 100),
        QColor(200, 100, 255)
    };

    for (const auto& c : palette)
    {
        bool used = false;

        if (structureSet)
        {
            for (const auto& s : structureSet->structures)
            {
                if (s.color == c)
                {
                    used = true;
                    break;
                }
            }
        }

        if (!used)
            return c;
    }

    int n = structureSet
        ? static_cast<int>(structureSet->structures.size())
        : 0;

    return QColor::fromHsv((n * 47) % 360, 255, 255);
}
