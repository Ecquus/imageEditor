#include "dataaccess.h"

#include <QDebug>
#include <memory>

const QImage::Format IDataAccess::imageFormat{ QImage::Format_ARGB32 };

auto DataAccess::loadImage(const QString& filepath) -> std::optional<QImage>
{
    auto img = QImage(filepath).mirrored().convertToFormat(IDataAccess::imageFormat);

    if (img.isNull())
        return {};

    history = std::make_unique<History>(img);
    return img;
}

void DataAccess::saveImage(const QString& filepath) const
{
    history->back().mirrored().save(filepath);
}

auto DataAccess::getImage() const -> std::optional<QImage>
{
    if (!history)
        return {};
    
    return history->current();
}

auto DataAccess::undo() -> std::optional<QImage>
{
    if (!history)
        return {};
    
    return history->undo();
}

auto DataAccess::redo() -> std::optional<QImage>
{
    if (!history)
        return {};
    
    return history->redo();
}

void DataAccess::appendHistory(QImage image)
{
    if (!history)
        return;

    history->append(image);
}
