#include "coordconverter.h"
#include <util.h>

using namespace util::types;

auto CoordConverter::toPixelCoord(int height, const QPoint& p) -> QPoint
{
    return { p.x(), height - 1 - p.y() };
}

auto CoordConverter::pixelToNormalized(float width, float height, const QPoint& pixel) -> QVector3D
{
    const auto halfw = toFloat(util::floor(width / 2.0f));
    const auto halfh = toFloat(util::floor(height / 2.0f));

    const auto aspect = width / height;

    const auto xn = (toFloat(pixel.x()) - halfw) / halfw;
    const auto yn = (toFloat(pixel.y()) - halfh) / halfh;

    return { aspect * xn, yn, 0.0f };
}

auto CoordConverter::normalizedToPixel(float width, float height, const QVector3D& norm) -> QPoint
{
    const auto halfw = toFloat(util::floor(width / 2.0f));
    const auto halfh = toFloat(util::floor(height / 2.0f));

    const auto aspect = width / height;
    const auto x = util::round(norm.x() / aspect * halfw + halfw);
    const auto y = util::round(norm.y() * halfh + halfh);

    return { x, y };
}
