#include "displaysettingsmanager.h"

#include <QMatrix4x4>

auto DisplaySettingsManager::getZoom() const -> float
{ 
    return zoom; 
}

void DisplaySettingsManager::setZoom(float value)    
{ 
    zoom = value; 
}

void DisplaySettingsManager::beginRotation(const QVector3D& origin, const QVector3D& layerTranslate, float angle)
{
    origLayerClick = origin - layerTranslate;
    origLayerRotate = angle;
}

auto DisplaySettingsManager::getRotationVector() const -> QVector3D
{
    auto rot = QMatrix4x4{};
    rot.rotate(-origLayerRotate, 0.0f, 0.0f, 1.0f);
    return rot * origLayerClick;
}

void DisplaySettingsManager::beginLayerMovement(const QVector3D& origin, const QVector3D& layerTranslate)
{
    origLayerClick = origin - layerTranslate;
}

auto DisplaySettingsManager::getLayerMovementOrigin() const -> QVector3D
{
    return origLayerClick;
}

void DisplaySettingsManager::beginCameraMovement(const QVector3D& origin)
{
    origCamClick = origin;
}

void DisplaySettingsManager::moveCameraFromOrigin(const QVector3D& click)
{
    camOffset = origCamClick - click;
}

void DisplaySettingsManager::endCameraMovement()
{
    origCamOffset += camOffset;
    camOffset = {};
}

auto DisplaySettingsManager::getCameraTranslate() const -> QVector3D
{
    return (-origCamOffset - camOffset) * getZoom();
}

void DisplaySettingsManager::resetSettings()
{
    origLayerClick = {};
    origLayerRotate = 0.0f;
    origCamClick = {};
    camOffset = {};
    origCamOffset = {};
}

