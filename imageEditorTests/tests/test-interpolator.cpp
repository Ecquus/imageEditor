#include <catch.hpp>
#include <qimage.h>
#include <interpolator.h>

TEST_CASE("Test linear interpolation", "[interpolator/linear]")
{
    const auto toQRgb = [](int k) -> QRgb { return qRgba(k, k, k, 0xff); };

    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 0.00f) == toQRgb(100));
    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 0.25f) == toQRgb(125));
    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 0.33f) == toQRgb(133));
    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 0.50f) == toQRgb(150));
    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 0.66f) == toQRgb(166));
    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 0.75f) == toQRgb(175));
    CHECK(Interpolator::linear(toQRgb(100), toQRgb(200), 1.00f) == toQRgb(200));
}

TEST_CASE("Test nearest neighbour interpolation", "[interpolator/nearest]")
{
    const auto toQRgb = [](int k) -> QRgb { return qRgba(k, k, k, 0xff); };

    auto img = QImage{ 2, 2, QImage::Format_ARGB32 };
    img.setPixel(0, 0, toQRgb(10));
    img.setPixel(1, 0, toQRgb(20));
    img.setPixel(0, 1, toQRgb(30));
    img.setPixel(1, 1, toQRgb(40));

    CHECK(Interpolator::nearest(Cell{ QPointF{ 0.25f, 0.25f }, img, 0x00 }) == toQRgb(10));
    CHECK(Interpolator::nearest(Cell{ QPointF{ 0.75f, 0.25f }, img, 0x00 }) == toQRgb(20));
    CHECK(Interpolator::nearest(Cell{ QPointF{ 0.25f, 0.75f }, img, 0x00 }) == toQRgb(30));
    CHECK(Interpolator::nearest(Cell{ QPointF{ 0.75f, 0.75f }, img, 0x00 }) == toQRgb(40));
}

TEST_CASE("Test bilinear interpolation", "[interpolator/bilinear]")
{
    const auto toQRgb   = [](int k) -> QRgb { return qRgba(k, k, k, 0xff); };

    auto img = QImage{ 2, 2, QImage::Format_ARGB32 };
    img.setPixel(0, 0, toQRgb(0));
    img.setPixel(1, 0, toQRgb(100));
    img.setPixel(0, 1, toQRgb(100));
    img.setPixel(1, 1, toQRgb(200));
    
    CHECK(Interpolator::bilinear(Cell{ QPointF{ 0.25f, 0.25f }, img, 0x00 }) == toQRgb(50));
    CHECK(Interpolator::bilinear(Cell{ QPointF{ 0.75f, 0.25f }, img, 0x00 }) == toQRgb(100));
    CHECK(Interpolator::bilinear(Cell{ QPointF{ 0.25f, 0.75f }, img, 0x00 }) == toQRgb(100));
    CHECK(Interpolator::bilinear(Cell{ QPointF{ 0.75f, 0.75f }, img, 0x00 }) == toQRgb(150));
}
