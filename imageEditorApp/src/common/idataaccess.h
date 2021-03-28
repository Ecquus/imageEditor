#pragma once

#include <optional>
#include <QImage>
#include <QString>

class IDataAccess
{
public:
    static const QImage::Format imageFormat;

    virtual ~IDataAccess() { }

    virtual auto loadImage(const QString& filepath) -> std::optional<QImage> = 0;
    virtual void saveImage(const QString& filepath) const = 0;
    virtual auto getImage() const -> std::optional<QImage> = 0;
    virtual void appendHistory(QImage image) = 0;
    virtual auto undo() -> std::optional<QImage> = 0;
    virtual auto redo() -> std::optional<QImage> = 0;
};
