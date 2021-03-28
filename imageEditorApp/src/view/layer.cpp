#include "layer.h"
#include <logger.h>
#include "coordconverter.h"
#include <util.h>

#include <cassert>
#include <cmath>
#include <vector>
#include <QOpenGLTexture>
#include <QOpenGLPixelTransferOptions>
#include <QPainter>
#include <QVector3D>

std::shared_ptr<VertexBuffer>   LayerBase::defaultVbo{ nullptr };
const float                     LayerBase::defaultScale{ 0.75f };
util::owner_ptr<QOpenGLTexture> UpperLayer::overlayTexture{ nullptr };
const QRgb                      FrameLayer::borderColor{ qRgba(0x80, 0x80, 0x80, 0x00) };

auto LayerBase::layerRectFromWinRect(const QRect& winrect) const -> QRect
{
    return QRect{ layerCoordFromWinCoord(winrect.topLeft()), winrect.size() };
}

auto LayerBase::layerCoordFromWinCoord(const QPoint& wincoord) const -> QPoint
{
    // normalize clicked position to the window's dimensions
    const auto normvec = display.pixelToNormalized(wincoord);

    // apply a reverse transformation to this vector to obtain the original normalized
    // coordinates, as if the layer's corners were the same as the corners of the display
    const auto scaleVec = getScale();
    auto revtrans = QMatrix4x4{};
    revtrans.scale(1.0f / scaleVec.x(), 1.0f/ scaleVec.y());
    revtrans.rotate(-rotateDelta, 0.0f, 0.0f, 1.0f);
    revtrans.translate(-getTranslate().x(), -getTranslate().y());

    const auto revnormvec = revtrans * normvec;

    // the size of one pixel in the normalized coordinate system
    const auto sx = 2.0f / getWidthF();
    const auto sy = 2.0f / getHeightF();

    // get the actual coordinates in the image
    const auto x = util::floor((revnormvec.x() + 1.0f) / sx);
    const auto y = util::floor((revnormvec.y() + 1.0f) / sy);

    return { x, y };
}

auto LayerBase::texCoordFromWinCoord(const QPoint& winCoord) const -> QPointF
{
    const auto v = layerCoordFromWinCoord(winCoord);
    return { toFloat(v.x()) / getWidthF(), toFloat(v.y()) / getHeightF() };
}

// TODO: add QPoint{ 1, 1 } *conditionally* if the normalized coordinate is
//       in the negative range and there is room for one more pixel (one image pixel
//       is represented with 2 or more display pixels)
//       for now, QPoint{ 1, 1 } is added for a temporary workaround
auto LayerBase::getWinRect() const -> QRect
{
    const auto winCenter = display.normalizedToPixel(getTranslate());

    const auto halfWidth  = util::floor(getWidthF() * display.getZoom() / 2.0f);
    const auto halfHeight = util::floor(getHeightF() * display.getZoom() / 2.0f);

    const auto leftTop = QPoint{ winCenter.x() - halfWidth,
        winCenter.y() - halfHeight } + QPoint{ 1, 1 };

    return QRect{ leftTop, QSize{ getWidth(), getHeight() }};
}

auto LayerBase::getZoomedWinRect() const -> QRect
{
    const auto zoom = display.getZoom();
    const auto rect = getWinRect();

    return QRect{ rect.topLeft(), QSize{ util::round(getWidthF() * zoom),
            util::round(getHeightF() * zoom) }};
}

auto UpperLayer::getScale() const -> QVector3D
{
    const auto v = QVector3D{ getWidthF(), getHeightF(), 0.0f };
    return (display.getZoom() * v) / display.getHeightF();
}

auto UpperLayer::getImage() const -> std::optional<QImage>
{
    assert(!(copyData && cutData));

    if (copyData)     return copyData->image;
    else if (cutData) return cutData->image;
    else              return std::nullopt;
}

void UpperLayer::setCopyData(const QRect& sourcePosition, QImage image,
                             std::shared_ptr<VertexBuffer> vbo, QOpenGLTexture* texture)
{
    if (!texture)
    {
        Logger::error("Tried to set upperLayer copy texture with a null pointer!");
        return;
    }
    
    this->vbo = vbo;
    this->texture = texture;
    copyData = std::make_unique<CopyData>(image, sourcePosition);
}

void UpperLayer::setCutData(IDisplay& display, const QImage& image,
                            const QRect& sourcePosition, util::owner_ptr<QOpenGLTexture> texture)
{
    this->texture = texture.get();
    cutData = std::make_unique<CutData>(display, image, sourcePosition, std::move(texture));
}

auto BackgroundLayer::getScale() const -> QVector3D
{
    const auto visibleHeightRatio = (getHeightF() * display.getZoom()) / display.getHeightF();
    return { getAspect() * visibleHeightRatio, visibleHeightRatio, 0.0f };
}

// TODO: loading textures this way may not be portable; should find a portable way of dealing
//       with pixel byte order and endianness
// TODO: put transparent pixels into a large static buffer, and send texture data from there
// TODO: fix setData instead of creating a new texture
void BackgroundLayer::eraseArea(const QRect& rect)
{
    const auto black = qRgba(0x00, 0x00, 0x00, 0xff);
    const auto buf = std::vector<QRgb>(std::size_t(rect.width() * rect.height()), black);

    auto painter = QPainter{ &image };
    painter.setBackground(QBrush{ black });
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.eraseRect(rect);

    texture.reset();
    texture = display.makeTexture(image);
    // texture->setData(rect.left(), rect.top(), 0,
    //                  rect.width(), rect.height(), 0,
    //                  QOpenGLTexture::PixelFormat::BGRA, QOpenGLTexture::UInt32_RGBA8_Rev, buf.data());
}

void BackgroundLayer::fillArea(const CopyData& data)
{
    auto painter = QPainter{ &image };
    painter.setBackground(QBrush{ Qt::gray });
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.drawImage(data.sourcePosition, data.image);
    
    texture->setData(data.sourcePosition.left(), data.sourcePosition.topLeft().y(), 0,
                     data.image.width(), data.image.height(), 0,
                     QOpenGLTexture::PixelFormat::BGRA, QOpenGLTexture::UInt32_RGBA8_Rev, data.image.constBits());
}

FrameLayer::FrameLayer(IDisplay& display, int width, int height)
    : LayerBase{ display, LayerBase::defaultVbo }
    , texture{ [&]{
        auto img = QImage{ width, height, QImage::Format_ARGB32 };
        img.fill(qRgba(0x00, 0x00, 0x00, 0x00));
        
        auto painter = QPainter{ &img };
        painter.setPen(QPen{ QBrush{ borderColor }, 4 });
        painter.drawRect(QRect{ QPoint{ 1, 1 }, QPoint{ width - 2, height - 2 }});

        return util::make_owner<QOpenGLTexture>(img);
    }() }
{
}

auto FrameLayer::getScale() const -> QVector3D
{
    const auto v = QVector3D{ getWidthF(), getHeightF(), 0.0f };
    return (display.getZoom() * v) / display.getHeightF();
}
