#ifdef _WIN32
#  define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "displaywidget.h"
#include "coordconverter.h"
#include <logger.h>
#include "openglexception.h"
#include <util.h>

#include <array>
#include <limits>
#include <optional>
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QPoint>
#include <QPainter>
#include <QPolygon>
#include <QTransform>

const int   DisplayWidget::vertexAttribLoc{ 0 };
const int   DisplayWidget::texcoordAttribLoc{ 1 };
const float DisplayWidget::zoomStep{ 0.25f };
const uint  DisplayWidget::defaultGray{ 0xbc };
const QRgb  DisplayWidget::darkOverlayColor{ qRgba(0x00, 0x00, 0x00, 0x80) };
const QRgb  DisplayWidget::lightOverlayColor{ qRgba(0xbc, 0xbc, 0xbc, 0x80) };

DisplayWidget::DisplayWidget(QWidget* parent, IMainWindow& mainWindow)
    : QOpenGLWidget{ parent }
    , mainWindow{ mainWindow }
{
    setMouseTracking(true);
}

DisplayWidget::~DisplayWidget()
{
    // OpenGL objects must be released in the current context
    makeCurrent();

    frameLayer.reset();
    UpperLayer::overlayTexture.reset();
    backgroundLayer.reset();
    upperLayer.reset();

    doneCurrent();
}

void DisplayWidget::initializeGL()
{
    initializeOpenGLFunctions();
    
    // specifiy OpenGL options
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }
    
    // init shaders
    {
        shaderProgram = makeShader(vertexShaderSource, fragmentShaderSource);
        shaderProgram->setUniformValue("texture1", 0);
        shaderProgram->setUniformValue("redLevel", 0.5f);
        shaderProgram->setUniformValue("greenLevel", 0.5f);
        shaderProgram->setUniformValue("blueLevel", 0.5f);
        shaderProgram->setUniformValue("brightLevel", 0.0f);
        shaderProgram->setUniformValue("contrastLevel", 1.0f);

        grayShaderProgram = makeShader(vertexShaderSource, grayscaleFragmentShaderSource);

        selectedShader = shaderProgram.get();
    }
    
    // setup vertex data
    LayerBase::defaultVbo = VertexBuffer::make();

    setOverlayColor(darkOverlayColor);
}

void DisplayWidget::paintGL()
{
    glClearColor(toFloat(defaultGray) / 255.0f, toFloat(defaultGray) / 255.0f, toFloat(defaultGray) / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawLayers();
}

void DisplayWidget::mousePressEvent(QMouseEvent* event)
{
    if (!backgroundLayer)
        return;

    // cancel the active selection if the user presses the mouse
    if (upperLayer && upperLayer->inSelectMode())
    {
        revertChanges();
        return;
    }

    const auto click     = toPixelCoord(event->pos());
    const auto normClick = pixelToNormalized(click);

    // the background layer is not selectable if an upper layer exists
    const auto layer = layerAtPoint(adjustToCamera(click));
    if (upperLayer && layer == backgroundLayer.get())
        return;

    selectedLayer = layer;
    if (selectedLayer)
    {
        if (event->buttons() & Qt::LeftButton)
            displaySettingsMgr.beginLayerMovement(normClick, selectedLayer->getTranslate());
        else if (event->buttons() & Qt::MiddleButton)
            displaySettingsMgr.beginCameraMovement(normClick);
        else if (event->buttons() & Qt::RightButton)
            displaySettingsMgr.beginRotation(normClick, selectedLayer->getTranslate(), selectedLayer->getRotate());
    }
    else
    {
        displaySettingsMgr.beginRotation(normClick, backgroundLayer->getTranslate(), backgroundLayer->getRotate());
    }

    emit displayFocused();
}

void DisplayWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!backgroundLayer)
        return;

    const auto pixClick = toPixelCoord(event->pos());
    auto normClick = pixelToNormalized(pixClick);

    // LMB selects or moves a layer
    if (event->buttons() & Qt::LeftButton)
    {
        auto lastClick = displaySettingsMgr.getLayerMovementOrigin();
        
        // shift + LMB starts selection mode
        if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            // adjust normClick to camera offset
            lastClick = adjustToCamera(lastClick);
            normClick = adjustToCamera(normClick);
            
            // selection is not allowed if the background layer is translated or rotated,
            // or if there is an ongoing cut or copy operation with upperLayer
            if (backgroundLayer->isRotated() || backgroundLayer->isTranslated() || (upperLayer && !upperLayer->inSelectMode()))
                return;

            if (!upperLayer)
                upperLayer = util::make_owner<UpperLayer>(*this);

            // width of the image in normalized form
            const auto normImageWidth  = (2.0f * getAspect() * backgroundLayer->getWidthF()) / getWidthF() * getZoom();
            const auto normImageHeight = (2.0f * backgroundLayer->getHeightF()) / getHeightF() * getZoom();

            // get the size of an image pixel in normalized form
            const auto normPixelSize = normImageWidth / backgroundLayer->getWidthF();

            // left top position of the background image in the display in normalized form
            const auto normImageLeftTop = QPointF {
                ((2.0f * getAspect() - normImageWidth) / 2.0f) - getAspect(),
                ((2.0f - normImageHeight) / 2.0f) - 1.0f
            };

            // lambdas to easily calculate the left top and the bottom right of an image pixel in normalized form
            const auto getNormLeftTop = [&](const QPointF& norm) {
                const auto ltx = toFloat(normImageLeftTop.x()), lty = toFloat(normImageLeftTop.x());

                const auto x = std::floor((norm.x() - ltx) / normPixelSize) * normPixelSize + ltx;
                const auto y = std::floor((norm.y() - lty) / normPixelSize) * normPixelSize + lty;

                return QPointF{ x, y };
            };
            const auto getNormRightBot = [&](const QPointF& norm) {
                const auto ltx = toFloat(normImageLeftTop.x()), lty = toFloat(normImageLeftTop.x());

                const auto x = std::ceil((norm.x() - ltx) / normPixelSize) * normPixelSize + ltx;
                const auto y = std::ceil((norm.y() - lty) / normPixelSize) * normPixelSize + lty;

                return QPointF{ x, y };
            };

            // calculate the left top and bottom right corners of the selection rect
            const auto leftTopPoint  = QPointF{ std::min(lastClick.x(), normClick.x()), std::min(lastClick.y(), normClick.y()) };
            const auto rightBotPoint = QPointF{ std::max(lastClick.x(), normClick.x()), std::max(lastClick.y(), normClick.y()) };

            const auto normLeftTop  = getNormLeftTop(leftTopPoint);
            const auto normRightBot = getNormRightBot(rightBotPoint);
            const auto normCenter   = QVector3D{ toFloat(normLeftTop.x() + normRightBot.x()) / 2.0f,
                                                 toFloat(normLeftTop.y() + normRightBot.y()) / 2.0f, 0.0f };
            upperLayer->translateTo(normCenter);

            // calculate the dimensions of the selection rect
            const auto selectWidth  = util::round(std::abs(normLeftTop.x() - normRightBot.x()) / normPixelSize);
            const auto selectHeight = util::round(std::abs(normLeftTop.y() - normRightBot.y()) / normPixelSize);
            const auto selectSize   = QSize{ selectWidth, selectHeight };
            upperLayer->setSelectSize(selectSize);

            const auto selectRect = frameLayer->layerRectFromWinRect(upperLayer->getWinRect());
            Logger::debug(QString{ "leftTopNorm is "} + util::toQString(normLeftTop) +
                             "; rightBotNorm is " + util::toQString(normRightBot) +
                             "; center is " + util::toQString(normalizedToPixel(normCenter)) +
                             "; selectRect is " + util::toQString(selectRect) +
                          "; selectLeftTop is " + util::toQString(selectRect.topLeft()));
        }
        else if (selectedLayer)
        {
            selectedLayer->translateTo(normClick - lastClick);
        }

        emit actionStarted();
    }
    // MMB moves the camera
    else if (event->buttons() & Qt::MiddleButton && selectedLayer)
    {
        displaySettingsMgr.moveCameraFromOrigin(normClick);
    }
    // RMB starts rotation
    else if (event->buttons() & Qt::RightButton)
    {
        const auto layer = (selectedLayer) ? selectedLayer : backgroundLayer.get();

        const auto a = normalizedToPixel(displaySettingsMgr.getRotationVector());
        const auto b = pixClick;
        const auto c = normalizedToPixel(layer->getTranslate());

        const auto alpha = toFloat(std::atan2(b.x() - c.x(), b.y() - c.y()));
        const auto beta  = toFloat(std::atan2(a.x() - c.x(), a.y() - c.y()));
        const auto gamma = beta - alpha;

        Logger::debug(QString{ "Calculating rotation: a is " } + util::toQString(a) +
                         ", b is " + util::toQString(b) + ", c is " + util::toQString(c) +
                         ", rad(gamma) is " + QString::number(gamma) + " and deg(gamma) is " +
                         QString::number(gamma * 180.0f / toFloat(M_PI)));

        layer->rotateTo(gamma * 180.0f / toFloat(M_PI));

        emit actionStarted();
    }

    emit cursorPositionChanged(backgroundLayer->layerCoordFromWinCoord(adjustToCamera(pixClick)));
    
    update();
}

void DisplayWidget::mouseReleaseEvent(QMouseEvent* )
{
    displaySettingsMgr.endCameraMovement();
    update();
}

void DisplayWidget::wheelEvent(QWheelEvent* event)
{
    std::optional<float> zoom;
    if (event->angleDelta().y() > 0) // wheel up
        zoom = zoomIn();
    else if (event->angleDelta().y() < 0) // wheel down
        zoom = zoomOut();

    if (zoom)
        emit zoomChanged(*zoom);
}

auto DisplayWidget::toPixelCoord(const QPoint& p) -> QPoint
{
    return CoordConverter::toPixelCoord(getHeight(), p);
}

auto DisplayWidget::adjustToCamera(const QPoint& pixel) const -> QPoint
{
    const auto dispCenter = normalizedToPixel({ 0, 0, 0 });
    const auto camNormOffset = displaySettingsMgr.getCameraTranslate();
    const auto camPixOffset = dispCenter - normalizedToPixel(camNormOffset);

    return pixel + camPixOffset;
}

auto DisplayWidget::adjustToCamera(const QVector3D& norm) const -> QVector3D
{
    return norm - displaySettingsMgr.getCameraTranslate();
}

auto DisplayWidget::nextZoomLevel(float level) const -> float
{
    const auto a = toFloat(zoomStep / 2.0f);
    const auto b = toFloat(1.0f / zoomStep);

    return std::round((level + a) * b) / b;
}

auto DisplayWidget::prevZoomLevel(float level) const -> float
{
    const auto a = toFloat(zoomStep / 2.0f);
    const auto b = toFloat(1.0f / zoomStep);
    const auto r = toFloat((level - a) * b);

    // round wouldn't work when the value is exactly at x.5f
    // e.g. for 1.0f we would return 1.0f, for 0.75f 0.75f, etc.
    return std::ceil(r - 0.5f) / b;
}

auto DisplayWidget::makeShader(const char* vShaderSrc, const char* fShaderSrc) -> std::unique_ptr<QOpenGLShaderProgram>
{
    assert(vShaderSrc && "vShaderSrc must not be null!");
    assert(fShaderSrc && "fShaderSrc must not be null!");
    
    auto shader = std::make_unique<QOpenGLShaderProgram>();

    if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vShaderSrc))
        throw OpenGLException{ "Failed to add vertex shader, error: " + shader->log() };
    
    if (!shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fShaderSrc))
        throw OpenGLException{ "Failed to add fragment shader, error: " + shader->log() };

    shader->bindAttributeLocation("in_Position", vertexAttribLoc);
    shader->bindAttributeLocation("in_TexCoords", texcoordAttribLoc);

    if (!shader->link())
        throw OpenGLException{ "Failed to link shader program!" };
    
    if (!shader->bind())
        throw OpenGLException{ "Failed to bind shader program!" };

    return shader;
}

void DisplayWidget::forEachLayer(const std::function<void(LayerBase*)>& func)
{
    for (auto layer : std::array<LayerBase*, 3>{ frameLayer.get(), backgroundLayer.get(), upperLayer.get() })
        if (layer)
            func(layer);
}

auto DisplayWidget::layerAtPoint(const QPoint& point) const -> LayerBase*
{
    for (const auto layer : std::array<LayerBase*, 2>{ upperLayer.get(), backgroundLayer.get() })
    {
        if (!layer)
            continue;

        const auto halfW = toInt(std::round((layer->getWidthF() * getZoom()) / 2.0f));
        const auto halfH = toInt(std::round((layer->getHeightF() * getZoom()) / 2.0f));
        const auto center = normalizedToPixel(layer->getTranslate());

        const auto rot = QTransform()
            .translate(qreal(center.x()), qreal(center.y()))
            .rotate(-layer->getRotate())
            .translate(-qreal(center.x()), -qreal(center.y()));

        const auto rect = QRect{ QPoint{ center.x() - halfW, center.y() - halfH },
                                 QPoint{ center.x() + halfW, center.y() + halfH }};

        if (rot.mapToPolygon(rect).containsPoint(point, Qt::FillRule::OddEvenFill))
            return layer;
    }

    return nullptr;
}

void DisplayWidget::drawLayer(LayerBase& layer, const QMatrix4x4& matrix)
{
    if (!selectedShader->bind())
        throw OpenGLException{ "Failed to bind shader program during drawLayer!" };

    selectedShader->setUniformValue("matrix", matrix);
    selectedShader->enableAttributeArray(vertexAttribLoc);
    selectedShader->enableAttributeArray(texcoordAttribLoc);
    selectedShader->setAttributeBuffer(vertexAttribLoc, GL_FLOAT, 0, 2, 4 * sizeof(GLfloat));
    selectedShader->setAttributeBuffer(texcoordAttribLoc, GL_FLOAT, 2 * sizeof(GLfloat), 2, 4 * sizeof(GLfloat));

    if (selectedShader == shaderProgram.get())
    {
        const auto colorData = mainWindow.getColorData();
        selectedShader->setUniformValue("redLevel", colorData.red);
        selectedShader->setUniformValue("greenLevel", colorData.green);
        selectedShader->setUniformValue("blueLevel", colorData.blue);
        selectedShader->setUniformValue("brightLevel", colorData.bright);
        selectedShader->setUniformValue("contrastLevel", colorData.contrast);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    layer.bindTexture();

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DisplayWidget::drawLayers()
{
    forEachLayer([this](LayerBase* layer) {
        layer->bindVbo();

        auto projection = QMatrix4x4{};
        projection.setToIdentity();
        projection.ortho(-1.0f * getAspect(), getAspect(), -1.0f, +1.0f, -1.0f, +1.0f);

        auto transform = QMatrix4x4{};
        transform.setToIdentity();
        transform.translate(layer->getTranslate());
        transform.translate(displaySettingsMgr.getCameraTranslate());
        transform.rotate(layer->getRotate(), 0.0f, 0.0f, 1.0f);
        transform.scale(layer->getScale());
    
        drawLayer(*layer, projection * transform);
    });
}

auto DisplayWidget::imageFromDisplay() -> std::optional<QImage>
{
    if (!backgroundLayer)
        return std::nullopt;

    makeCurrent();

    const auto width = frameLayer->getWidth();
    const auto height = frameLayer->getHeight();
    
    glViewport(0, 0, width, height);
    QOpenGLFramebufferObject framebuffer{ width, height, GL_TEXTURE_2D };

    if (!framebuffer.bind())
        throw OpenGLException{ "Failed to bind framebuffer!" };
    
    forEachLayer([this](LayerBase* layer) {
        drawLayer(*layer);
    });
    
    const auto img = QImage{ framebuffer.toImage().mirrored() };

    if (!framebuffer.release())
        throw OpenGLException{ "Failed to release framebuffer!" };
    
    if (!framebuffer.bindDefault())
        throw OpenGLException{ "Failed to rebind default framebuffer!" };

    doneCurrent();

    return img;
}

auto DisplayWidget::pixelToNormalized(const QPoint& pixel) const -> QVector3D
{
    return CoordConverter::pixelToNormalized(getWidthF(), getHeightF(), pixel);
}

auto DisplayWidget::normalizedToPixel(const QVector3D& norm) const -> QPoint
{
    return CoordConverter::normalizedToPixel(getWidthF(), getHeightF(), norm);
}
    
auto DisplayWidget::makeTexture(const QImage& image) -> util::owner_ptr<QOpenGLTexture>
{
    makeCurrent();
    auto p = util::make_owner<QOpenGLTexture>(image);

    p->setMagnificationFilter(QOpenGLTexture::Nearest);
    p->setMinificationFilter(QOpenGLTexture::Nearest);

    doneCurrent();
    return p;
}

void DisplayWidget::deleteTexture(util::owner_ptr<QOpenGLTexture> texture)
{
    makeCurrent();
    texture.reset();
    doneCurrent();
}

auto DisplayWidget::getRotate() const -> float
{
    return selectedLayer ? selectedLayer->getRotate() : 0.0f;
}

auto DisplayWidget::getDefaultZoom() const -> float
{
    return (getHeightF() * LayerBase::defaultScale) / backgroundLayer->getHeightF();
}

auto DisplayWidget::getBackgroundImage() const -> std::optional<QImage>
{
    return backgroundLayer ? std::make_optional(backgroundLayer->getImage()) : std::nullopt;
}

void DisplayWidget::setOverlayColor(QRgb color)
{
    auto img = QImage{ 1, 1, QImage::Format_ARGB32 };
    img.fill(color);

    if (UpperLayer::overlayTexture)
        UpperLayer::overlayTexture.reset();

    UpperLayer::overlayTexture = makeTexture(img);
}

auto DisplayWidget::zoomIn() -> std::optional<float>
{
    if (!backgroundLayer)
        return {};

    Logger::debug(QString{ "Entered " } + __func__);

    const auto z = nextZoomLevel(getZoom());

    setZoom(z);

    Logger::debug("Zoom in: " + QString::number(z));

    update();

    return z;
}

auto DisplayWidget::zoomOut() -> std::optional<float>
{
    if (getZoom() <= zoomStep || !backgroundLayer)
        return {};

    Logger::debug(QString{ "Entered " } + __func__);

    const auto z = prevZoomLevel(getZoom());
    setZoom(z);

    Logger::debug("Zoom out: " + QString::number(z));
    Logger::debug("Width is " + QString::number(getWidth()) + " and height is " + QString::number(getHeight()));

    update();

    return z;
}

void DisplayWidget::setGrayscale(bool grayscale)
{
    selectedShader = grayscale ? grayShaderProgram.get() : shaderProgram.get();
}

void DisplayWidget::displayImage(const QImage& img)
{
    if (!frameLayer || img.width() != frameLayer->getWidth() || img.height() != frameLayer->getHeight())
        frameLayer.reset(new FrameLayer{ *this, img.width(), img.height() });

    backgroundLayer.reset(new BackgroundLayer{ *this, img });

    upperLayer.reset();

    selectedLayer = backgroundLayer.get();

    update();
}

auto DisplayWidget::mergeLayers() -> QImage
{
    assert(frameLayer);
    assert(backgroundLayer);

    Logger::debug(QString{ "Entered " } + __func__);

    Logger::debug("backRect is " + util::toQString(backgroundLayer->getWinRect()) + ", " +
                  "frameBackRect" + util::toQString(frameLayer->layerRectFromWinRect(backgroundLayer->getWinRect())));

    if (backgroundLayer->isTranslated() || backgroundLayer->isRotated())
    {
        auto lower = backgroundLayer->getImage();
        auto emptyImg = QImage{ lower.size(), lower.format() };
        emptyImg.fill(Qt::black);

        return mainWindow.mergeImages(emptyImg, lower, frameLayer->layerRectFromWinRect(backgroundLayer->getWinRect()),
                                      backgroundLayer->getRotate());
    }
    else if (upperLayer && !upperLayer->inSelectMode())
    {
        const auto winUpperRect   = upperLayer->getWinRect();
        const auto layerUpperRect = frameLayer->layerRectFromWinRect(winUpperRect);

        Logger::debug("upperWinRect is " + util::toQString(upperLayer->getWinRect()) +
                      ", and frameUpperRect is " + util::toQString(layerUpperRect));
            
        return mainWindow.mergeImages(backgroundLayer->getImage(), *upperLayer->getImage(), layerUpperRect,
                                      upperLayer->getRotate());
    }

    auto opt = imageFromDisplay();
    if (opt)
        return *opt;

    return backgroundLayer->getImage();
}

auto DisplayWidget::revertChanges() -> bool
{
    if (!backgroundLayer)
        return false;

    if (upperLayer)
    {
        if (!upperLayer->inSelectMode() && upperLayer->getCutData())
            backgroundLayer->fillArea(*upperLayer->getCutData());

        upperLayer.reset();
    }

    backgroundLayer->rotateTo(0.0f);
    backgroundLayer->translateTo({ 0.0f, 0.0f, 0.0f });

    displaySettingsMgr.resetSettings();

    selectedLayer = backgroundLayer.get();

    update();

    return true;
}

auto DisplayWidget::copy() -> bool
{
    if (!backgroundLayer || !upperLayer)
        return false;

    // calculate the texture coordinates based on the selection rect's corners, and create a VBO from it
    const auto zoomedLowerRect  = backgroundLayer->getZoomedWinRect();
    const auto zoomedUpperRect  = upperLayer->getZoomedWinRect();
    const auto zoomedSelectRect = zoomedLowerRect.intersected(zoomedUpperRect);

    // if the selection rect is not in the frame
    if (zoomedSelectRect.size() == QSize{ 0, 0 })
        return false;

    const auto texLeftTop     = backgroundLayer->texCoordFromWinCoord(zoomedSelectRect.topLeft());
    const auto texRightTop    = backgroundLayer->texCoordFromWinCoord(zoomedSelectRect.topRight());
    const auto texRightBottom = backgroundLayer->texCoordFromWinCoord(zoomedSelectRect.bottomRight());
    const auto texLeftBottom  = backgroundLayer->texCoordFromWinCoord(zoomedSelectRect.bottomLeft());

    const auto vbo = VertexBuffer::make({ texLeftTop, texRightTop, texRightBottom, texLeftBottom });

    // specify the intersected part of the lower image
    const auto layerLowerRect  = frameLayer->layerRectFromWinRect(backgroundLayer->getWinRect());
    const auto layerUpperRect  = frameLayer->layerRectFromWinRect(upperLayer->getWinRect());
    const auto layerSelectRect = layerLowerRect.intersected(layerUpperRect);

    const auto img = mainWindow.getImage()->copy(layerSelectRect);

    upperLayer->setCopyData(layerSelectRect, img, vbo, &backgroundLayer->getTexture());

    upperLayer->setSelectSize(layerSelectRect.size());

    selectedLayer = upperLayer.get();

    update();

    return true;
}

auto DisplayWidget::cut() -> bool
{
    if (!backgroundLayer || !upperLayer || upperLayer->getCutData())
        return false;

    // specify the intersected part of the lower image
    const auto layerLowerRect  = frameLayer->layerRectFromWinRect(backgroundLayer->getWinRect());
    const auto layerUpperRect  = frameLayer->layerRectFromWinRect(upperLayer->getWinRect());
    const auto layerSelectRect = layerLowerRect.intersected(layerUpperRect);

    // if the selection rect is not in the frame
    if (layerSelectRect.size() == QSize{ 0, 0 })
        return false;

    const auto img = mainWindow.getImage()->copy(layerSelectRect);

    upperLayer->setCutData(*this, img, layerSelectRect, makeTexture(img));

    // paint the copied area to a default background color
    backgroundLayer->eraseArea(layerSelectRect);

    upperLayer->setSelectSize(layerSelectRect.size());

    selectedLayer = upperLayer.get();

    update();

    return true;
}

auto DisplayWidget::rotate(float angle) -> bool
{
    if (!backgroundLayer || (upperLayer && upperLayer->inSelectMode()))
        return false;

    const auto layer = (selectedLayer) ? selectedLayer : backgroundLayer.get();
    layer->rotateTo(angle);
    update();

    return true;
}
