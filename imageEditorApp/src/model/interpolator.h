#pragma once

#include <QImage>

struct Cell
{
    QRgb  corners[2][2];
    float x;
    float y;

    explicit Cell(const QPointF& p, const QImage& img, QRgb extrapColor);

private:
    static inline auto getColor(const QPoint& p, const QImage& img, QRgb extrapColor) { 
        return img.rect().contains(p) ? img.pixel(p) : extrapColor; 
    }
};

class Interpolator
{
public:
    static auto linear(QRgb fa, QRgb fb, float q) -> QRgb;
    static auto nearest(const Cell& cell) -> QRgb;
    static auto bilinear(const Cell& cell) -> QRgb;
};
