#pragma once

#include <QVector3D>

class DisplaySettingsManager
{
public:                                             
    auto getZoom() const -> float;
    void setZoom(float value);
    
    void beginRotation(const QVector3D& origin, const QVector3D& layerTranslate, float angle);
    auto getRotationVector() const -> QVector3D;

    void beginLayerMovement(const QVector3D& origin, const QVector3D& layerTranslate);
    auto getLayerMovementOrigin() const -> QVector3D;

    void beginCameraMovement(const QVector3D& origin);
    void moveCameraFromOrigin(const QVector3D& click);
    void endCameraMovement();
    auto getCameraTranslate() const -> QVector3D;

    void resetSettings();
    
private:
    float     zoom;

    QVector3D origLayerClick;
    float     origLayerRotate;
    
    QVector3D origCamClick;
    QVector3D camOffset;
    QVector3D origCamOffset;
};
