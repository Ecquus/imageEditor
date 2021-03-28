#pragma once

#include "idisplay.h"
#include <util.h>
#include "vertexbuffer.h"

#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QRect>

using namespace util::types;

class LayerBase
{
public:
    static std::shared_ptr<VertexBuffer> defaultVbo;
    static const float                   defaultScale;

    // this class only serves as a base class for other classes, it is not constructible
    virtual ~LayerBase() { }
    LayerBase(const LayerBase&) = delete;
    LayerBase& operator=(const LayerBase&) = delete;
    LayerBase(LayerBase&&) = delete;
    LayerBase& operator=(LayerBase&&) = delete;

    virtual auto getWidth() const -> int            { return getTexture().width(); }
    virtual auto getHeight() const -> int           { return getTexture().height(); }
    virtual auto getWidthF() const -> float         { return toFloat(getWidth()); }
    virtual auto getHeightF() const -> float        { return toFloat(getHeight()); }
    virtual auto getAspect() const -> float         { return getWidthF() / getHeightF(); }
    
    virtual auto getTextureId() const -> uint       { return getTexture().textureId(); }
    virtual void bindTexture()                      { getTexture().bind(); }
    
    virtual auto getTranslate() const -> QVector3D  { return translateDelta * display.getZoom(); }
    virtual void translate(const QVector3D& amount) { translateDelta += amount; }
    virtual void translateTo(const QVector3D& dest) { translateDelta = dest / display.getZoom(); }
    
    virtual auto getRotate() const -> float         { return -rotateDelta; }
    virtual void rotateTo(float angle)              { this->rotateDelta = -angle; }
    virtual void bindVbo()                          { vbo->bindVbo(); }
    
    virtual auto getScale() const -> QVector3D = 0;

    virtual auto layerRectFromWinRect(const QRect& winrect) const -> QRect;
    virtual auto layerCoordFromWinCoord(const QPoint& wincoord) const -> QPoint;
    virtual auto texCoordFromWinCoord(const QPoint& wincoord) const -> QPointF;
    
    virtual auto getWinRect() const -> QRect;
    virtual auto getZoomedWinRect() const -> QRect;
    
    virtual auto isTranslated() const -> bool       { return translateDelta != QVector3D{ 0.0f, 0.0f, 0.0f }; }
    virtual auto isRotated() const -> bool          { return rotateDelta != 0.0f; }


protected:
    IDisplay&                     display;
    std::shared_ptr<VertexBuffer> vbo;
    QVector3D                     translateDelta;
    float                         rotateDelta;

    explicit LayerBase(IDisplay& display, std::shared_ptr<VertexBuffer> vbo)
        : display{ display }
        , vbo{ std::move(vbo) }
        , translateDelta{ 0.0f, 0.0f, 0.0f }
        , rotateDelta{ 0.0f }
    {
    }

    virtual auto getTexture() -> QOpenGLTexture& = 0;
    virtual auto getTexture() const -> const QOpenGLTexture& = 0;
};

struct CopyData
{
    QImage image;
    QRect  sourcePosition;
    
    explicit CopyData(const QImage& image, const QRect& sourcePosition)
    : image{ image }
    , sourcePosition{ sourcePosition } { }
};

struct CutData : public CopyData
{
    IDisplay&                 display;
    util::owner_ptr<QOpenGLTexture> texture;

    explicit CutData(IDisplay& display, const QImage& image, const QRect& sourcePosition, util::owner_ptr<QOpenGLTexture> texture)
        : CopyData{ image, sourcePosition }
        , display{ display }
        , texture{ std::move(texture) } { }
    
    ~CutData() { display.deleteTexture(std::move(texture)); }
};

class UpperLayer : public LayerBase
{
public:
    static util::owner_ptr<QOpenGLTexture> overlayTexture;
    
    explicit UpperLayer(IDisplay& display, std::shared_ptr<VertexBuffer> vbo = LayerBase::defaultVbo)
        : LayerBase{ display, std::move(vbo) }
        , texture{ overlayTexture.get() } { }

    // inherited via LayerBase
    virtual auto getTexture() -> QOpenGLTexture& override             { return *texture; }
    virtual auto getTexture() const -> const QOpenGLTexture& override { return *texture; };
    virtual auto getWidth() const -> int override                     { return selectSize.width(); }
    virtual auto getHeight() const -> int override                    { return selectSize.height(); }
    virtual auto getScale() const -> QVector3D override;

    auto getImage() const -> std::optional<QImage>;
    
    auto getCopyData() const -> const CopyData*                       { return copyData.get(); }
    auto getCutData() const -> const CutData*                         { return cutData.get(); }

    void setCopyData(const QRect& sourcePosition, QImage image, std::shared_ptr<VertexBuffer> vbo, QOpenGLTexture* texture);
    void setCutData(IDisplay&, const QImage& image, const QRect& sourcePosition, util::owner_ptr<QOpenGLTexture> texture);

    auto inSelectMode() const -> bool                                 { return !copyData && !cutData; }
    void setSelectLeftTop(const QPoint& pixel)                        { selectLeftTop = pixel; }
    void setSelectRightBottom(const QPoint& pixel)                    { selectRightBottom = pixel; }
    void setSelectSize(const QSize& size)                             { selectSize = size; }

private:
    QOpenGLTexture*                 texture;
    std::unique_ptr<const CopyData> copyData;
    std::unique_ptr<const CutData>  cutData;
    QPoint                          selectLeftTop;
    QPoint                          selectRightBottom;
    QSize                           selectSize;
};

class BackgroundLayer : public LayerBase
{
public:
    explicit BackgroundLayer(IDisplay& display, const QImage& image)
        : LayerBase{ display, LayerBase::defaultVbo }
        , image{ image }
        , texture{ display.makeTexture(image) } { }

    virtual ~BackgroundLayer() override                               { display.deleteTexture(std::move(texture)); }

    // inherited via LayerBase
    virtual auto getTexture() -> QOpenGLTexture& override             { return *texture; }
    virtual auto getTexture() const -> const QOpenGLTexture& override { return *texture; };
    virtual auto getScale() const -> QVector3D override;
    
    auto getImage() const -> QImage                                   { return image; }
    void eraseArea(const QRect& rect);
    void fillArea(const CopyData& data);
    
private:
    QImage                          image;
    util::owner_ptr<QOpenGLTexture> texture;
};

class FrameLayer : public LayerBase
{
public:
    static const QRgb borderColor;

    explicit FrameLayer(IDisplay& display, int width, int height);
    
    virtual ~FrameLayer() override                                    { display.deleteTexture(std::move(texture)); }

    // inherited via LayerBase
    virtual auto getTexture() -> QOpenGLTexture& override             { return *texture; }
    virtual auto getTexture() const -> const QOpenGLTexture& override { return *texture; }
    virtual auto getScale() const -> QVector3D override;

private:
    util::owner_ptr<QOpenGLTexture> texture;
};
