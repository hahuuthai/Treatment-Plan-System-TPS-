#include "SliceRenderer.h"
#include "RTSTRUCT/StructureSetContainer.h"
#include <QPainter>
#include <QPen>
#include <QPolygon>
#include <algorithm>
#include <cmath>

QImage SliceRenderer::renderSlice(
    const CTVolume& volume,
    int z,
    double minLimit,
    double maxLimit,
    const StructureSetContainer* structureSet)
{
    int w = volume.width;
    int h = volume.height;

    QImage img(w, h, QImage::Format_RGB32);

    double range = std::max(1e-5, maxLimit - minLimit);

    // ===== CT GRAYSCALE =====
    for (int y = 0; y < h; y++)
    {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));

        for (int x = 0; x < w; x++)
        {
            int idx = z * w * h + y * w + x;
            double hu = volume.voxels[idx];

            int gray = static_cast<int>((hu - minLimit) * 255.0 / range);
            gray = qBound(0, gray, 255);

            line[x] = qRgb(gray, gray, gray);
        }
    }

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // ===== OVERLAY STRUCTURES =====
    if (structureSet)
    {
        for (const auto& structure : structureSet->structures)
        {
            if (!structure.visible)
                continue;

            QPen pen(structure.color);
            pen.setWidth(2);
            painter.setPen(pen);

            // ----- Case 1: vẽ từ contour gốc (RTSTRUCT) -----
            if (!structure.contours.empty())
            {
                for (const auto& contour : structure.contours)
                {
                    int nearestSlice = -1;
                    double bestDist = 1e9;

                    for (int s = 0; s < volume.depth; s++)
                    {
                        double dz = std::abs(contour.sliceZ - volume.slicePositions[s][2]);

                        if (dz < bestDist)
                        {
                            bestDist = dz;
                            nearestSlice = s;
                        }
                    }

                    if (nearestSlice != z)
                        continue;

                    QPolygon polygon;

                    for (const auto& p : contour.points)
                    {
                        double vx = p.x - volume.origin[0];
                        double vy = p.y - volume.origin[1];
                        double vz = p.z - volume.origin[2];

                        double col =
                            vx * volume.direction[0] +
                            vy * volume.direction[1] +
                            vz * volume.direction[2];

                        double row =
                            vx * volume.direction[3] +
                            vy * volume.direction[4] +
                            vz * volume.direction[5];

                        col /= volume.spacing[0];
                        row /= volume.spacing[1];

                        polygon << QPoint(qRound(col), qRound(row));
                    }

                    if (polygon.size() >= 2)
                        painter.drawPolyline(polygon);
                }
            }
            // ----- Case 2: vẽ viền từ mask -----
            else if (!structure.mask.voxels.empty())
            {
                for (int yy = 0; yy < structure.mask.height; yy++)
                {
                    for (int xx = 0; xx < structure.mask.width; xx++)
                    {
                        if (!structure.mask.at(xx, yy, z))
                            continue;

                        bool edge = false;

                        if (xx == 0) edge = true;
                        else if (!structure.mask.at(xx - 1, yy, z)) edge = true;

                        if (xx == structure.mask.width - 1) edge = true;
                        else if (!structure.mask.at(xx + 1, yy, z)) edge = true;

                        if (yy == 0) edge = true;
                        else if (!structure.mask.at(xx, yy - 1, z)) edge = true;

                        if (yy == structure.mask.height - 1) edge = true;
                        else if (!structure.mask.at(xx, yy + 1, z)) edge = true;

                        if (edge)
                            painter.drawPoint(xx, yy);
                    }
                }
            }
        }
    }

    painter.end();
    return img;
}

QString SliceRenderer::getOrientationLabel(const std::array<double, 3>& v)
{
    double x = v[0];
    double y = v[1];
    double z = v[2];

    QString label;

    if (x > 0.5) label += "L";
    else if (x < -0.5) label += "R";

    if (y > 0.5) label += "P";
    else if (y < -0.5) label += "A";

    if (z > 0.5) label += "H";
    else if (z < -0.5) label += "F";

    return label;
}
