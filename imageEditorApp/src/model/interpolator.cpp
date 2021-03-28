#include "interpolator.h"
#include <util.h>

using namespace util::types;

Cell::Cell(const QPointF& p, const QImage& img, QRgb extrapColor)
{
    // fill data with the colors of the pixels around point p in the image
    corners[0][0] = getColor(QPoint{ util::floor(p.x()), util::floor(p.y()) }, img, extrapColor);  // leftTop
    corners[0][1] = getColor(QPoint{ util::ceil(p.x()) , util::floor(p.y()) }, img, extrapColor);  // rightTop
    corners[1][0] = getColor(QPoint{ util::floor(p.x()), util::ceil(p.y())  }, img, extrapColor);  // leftBottom
    corners[1][1] = getColor(QPoint{ util::ceil(p.x()) , util::ceil(p.y())  }, img, extrapColor);  // rightBotom

    // normalize x and y to a range [0, 1)
    const auto px = toFloat(p.x()), py = toFloat(p.y());
    x = px - floorf(px);
    y = py - floorf(py);
}

// linear interpolation of f(q) from f(a) and f(b)
auto Interpolator::linear(QRgb fa, QRgb fb, float q) -> QRgb
{
    const auto ar = toFloat(qRed(fa)), ag = toFloat(qGreen(fa)), ab = toFloat(qBlue(fa));
    const auto br = toFloat(qRed(fb)), bg = toFloat(qGreen(fb)), bb = toFloat(qBlue(fb));

    assert(ar <= 255 && ag <= 255 && ab <= 255);
    assert(br <= 255 && bg <= 255 && bb <= 255);
        
    const auto r = util::clamp(util::round(ar * (1.0f - q) + br * q));
    const auto g = util::clamp(util::round(ag * (1.0f - q) + bg * q));
    const auto b = util::clamp(util::round(ab * (1.0f - q) + bb * q));

    return qRgba(r, g, b, 0xff);
}

auto Interpolator::nearest(const Cell& cell) -> QRgb
{
    const auto i = util::round(cell.x);
    const auto j = util::round(cell.y);
    return cell.corners[j][i];
}

auto Interpolator::bilinear(const Cell& cell) -> QRgb
{
    // linear interpolation along the x axis
    const auto fx1 = linear(cell.corners[0][0], cell.corners[0][1], cell.x);
    const auto fx2 = linear(cell.corners[1][0], cell.corners[1][1], cell.x);

    // linear interpolation along the y axis
    const auto fxy = linear(fx1, fx2, cell.y);

    return fxy;
}
