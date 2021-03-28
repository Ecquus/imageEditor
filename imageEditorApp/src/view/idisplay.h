#pragma once

#include <util.h>

class QOpenGLTexture;
class QPoint;
class QVector3D;

class IDisplay
{
public:
    virtual ~IDisplay() { }

    virtual auto getWidth() const -> int = 0;
    virtual auto getHeight() const -> int = 0;
    virtual auto getWidthF() const -> float = 0;
    virtual auto getHeightF() const -> float = 0;
    virtual auto getAspect() const -> float = 0;
    virtual auto getZoom() const -> float = 0;
    virtual auto pixelToNormalized(const QPoint& pixel) const -> QVector3D = 0;
    virtual auto normalizedToPixel(const QVector3D& norm) const -> QPoint = 0;
    virtual auto makeTexture(const QImage& image) -> util::owner_ptr<QOpenGLTexture> = 0;
    virtual void deleteTexture(util::owner_ptr<QOpenGLTexture> texture) = 0;
};
