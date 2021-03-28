#pragma once

#include <QPoint>
#include <QVector3D>

class CoordConverter
{
public:
    static auto toPixelCoord(int height, const QPoint& p) -> QPoint;
    static auto pixelToNormalized(float width, float height, const QPoint& pixel) -> QVector3D;
    static auto normalizedToPixel(float width, float height, const QVector3D& norm) -> QPoint;
};
