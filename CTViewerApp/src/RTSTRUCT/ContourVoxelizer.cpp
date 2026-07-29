#include "ContourVoxelizer.h"

#include <QImage>
#include <QPainter>
#include <QPolygon>
#include <cmath>

static int findNearestSlice(
    const CTVolume& volume,
    double z)
{
    int best = 0;

    double bestDist = 1e9;

    for (int i = 0; i < volume.depth; i++)
    {
        double dz =
            std::abs(
                volume.slicePositions[i][2]
                - z);

        if (dz < bestDist)
        {
            bestDist = dz;
            best = i;
        }
    }

    return best;
}

MaskVolume ContourVoxelizer::createMask(
    const StructureModel& structure,
    const CTVolume& volume)
{
    MaskVolume mask;

    mask.width = volume.width;
    mask.height = volume.height;
    mask.depth = volume.depth;

    mask.voxels.resize(
        volume.width *
        volume.height *
        volume.depth,
        0);

    for (const auto& contour :
        structure.contours)
    {
        int slice =
            findNearestSlice(
                volume,
                contour.sliceZ);

        QImage img(
            volume.width,
            volume.height,
            QImage::Format_Grayscale8);

        img.fill(0);

        QPainter painter(&img);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);

        QPolygon poly;
        for (const auto& p : contour.points)
        {
            // Patient coordinate -> relative vector
            double vx = p.x - volume.origin[0];
            double vy = p.y - volume.origin[1];
            double vz = p.z - volume.origin[2];

            // project lên row direction
            double col =
                vx * volume.direction[0] +
                vy * volume.direction[1] +
                vz * volume.direction[2];

            // project lên column direction
            double row =
                vx * volume.direction[3] +
                vy * volume.direction[4] +
                vz * volume.direction[5];

            col /= volume.spacing[0];
            row /= volume.spacing[1];

            poly << QPoint(
                qRound(col),
                qRound(row));
        }

        if (poly.size() < 3)
            continue;
        painter.drawPolygon(poly);

        painter.end();

        for (int y = 0; y < volume.height; y++)
        {
            for (int x = 0; x < volume.width; x++)
            {
                if (img.pixelColor(x, y).red())
                {
                    mask.at(
                        x,
                        y,
                        slice) = 1;
                }
            }
        }
    }

    return mask;
}