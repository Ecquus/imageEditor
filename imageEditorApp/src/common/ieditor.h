#pragma once

#include <QImage>
#include <QObject>
#include <QString>
#include <QVector3D>
#include <optional>

class IEditor
{
public:
    virtual ~IEditor() { }

    enum InterpMethod { NEAREST, BILINEAR, COUNT };

    virtual auto loadImage(const QString& filepath) -> std::optional<QImage> = 0;
    virtual void saveImage(const QString& filepath) const = 0;
    virtual auto getImage() const -> std::optional<QImage> = 0;

    virtual void appendHistory(QImage image) = 0;
    virtual auto undo() -> std::optional<QImage> = 0;
    virtual auto redo() -> std::optional<QImage> = 0;

    virtual void setInterpolationMethod(InterpMethod value) = 0;
    virtual auto mergeImages(QImage lower, QImage upper, const QRect& upperRect, float upperAngle) -> QImage = 0;
};
