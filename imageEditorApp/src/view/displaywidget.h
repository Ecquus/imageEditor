#pragma once

#include "coordconverter.h"
#include "displaysettingsmanager.h"
#include "idisplay.h"
#include "imainwindow.h"
#include "layer.h"
#include <util.h>
#include "vertexbuffer.h"

#include <memory>
#include <optional>
#include <vector>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

using namespace util::types;

class DisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions, public virtual IDisplay
{
    Q_OBJECT
public:
    static const QRgb darkOverlayColor;
    static const QRgb lightOverlayColor;

    explicit DisplayWidget(QWidget* parent, IMainWindow& mainWindow);
    virtual ~DisplayWidget() override;

    // inherited via IDisplay
    virtual auto getZoom() const -> float override { return displaySettingsMgr.getZoom(); }
    
    auto getRotate() const -> float;
    auto getDefaultZoom() const -> float;
    auto getBackgroundImage() const -> std::optional<QImage>;

    void setZoom(float level) { displaySettingsMgr.setZoom(level); update(); }
    void setOverlayColor(QRgb color);

    auto zoomIn() -> std::optional<float>;
    auto zoomOut() -> std::optional<float>;

    void setGrayscale(bool grayscale);
    void displayImage(const QImage& img);
    auto mergeLayers() -> QImage;
    auto revertChanges() -> bool;
    auto copy() -> bool;
    auto cut() -> bool;
    auto rotate(float angle = 0.0f) -> bool;

signals:
    void zoomChanged(float zoom);
    void displayFocused();
    void cursorPositionChanged(const QPoint& rect);
    void actionStarted();

private:
    static const char* const vertexShaderSource;
    static const char* const fragmentShaderSource;
    static const char* const grayscaleFragmentShaderSource;
    static const uint        defaultGray;
    static const int         vertexAttribLoc;
    static const int         texcoordAttribLoc;
    static const float       zoomStep;

    IMainWindow&                          mainWindow;
    
    std::unique_ptr<QOpenGLShaderProgram> shaderProgram{ nullptr };
    std::unique_ptr<QOpenGLShaderProgram> grayShaderProgram{ nullptr };
    QOpenGLShaderProgram*                 selectedShader{ nullptr };
    
    util::owner_ptr<FrameLayer>           frameLayer{ nullptr };
    util::owner_ptr<BackgroundLayer>      backgroundLayer{ nullptr };
    util::owner_ptr<UpperLayer>           upperLayer{ nullptr };
    LayerBase*                            selectedLayer{ nullptr };

    DisplaySettingsManager                displaySettingsMgr;

    // inherited via QOpenGLWidget
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    
    // inherited via IDisplay
    virtual auto getWidth() const -> int override     { return width(); }
    virtual auto getHeight() const -> int override    { return height(); }
    virtual auto getWidthF() const -> float override  { return toFloat(getWidth()); }
    virtual auto getHeightF() const -> float override { return toFloat(getHeight()); }
    virtual auto getAspect() const -> float override  { return getWidthF() / getHeightF(); }
    
    virtual auto pixelToNormalized(const QPoint& pixel) const -> QVector3D override;
    virtual auto normalizedToPixel(const QVector3D& norm) const -> QPoint override;
    virtual auto makeTexture(const QImage& image) -> util::owner_ptr<QOpenGLTexture> override;
    virtual void deleteTexture(util::owner_ptr<QOpenGLTexture> texture) override;

    auto toPixelCoord(const QPoint& p) -> QPoint;
    auto adjustToCamera(const QPoint& pixel) const -> QPoint;
    auto adjustToCamera(const QVector3D& norm) const -> QVector3D;
    
    auto nextZoomLevel(float level) const -> float;
    auto prevZoomLevel(float level) const -> float;

    auto makeShader(const char* vShaderSrc, const char* fShaderSrc) -> std::unique_ptr<QOpenGLShaderProgram>;
    
    void forEachLayer(const std::function<void(LayerBase*)>& func);
    auto layerAtPoint(const QPoint& point) const -> LayerBase*;
    void drawLayer(LayerBase& layer, const QMatrix4x4& matrix = {});
    void drawLayers();
    auto imageFromDisplay() -> std::optional<QImage>;
};
