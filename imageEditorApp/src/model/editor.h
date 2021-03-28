#pragma once

#include <ieditor.h>
#include <dataaccessfactory.h>
#include "interpolator.h"

#include <functional>
#include <optional>
#include <qimage.h>

class Editor : public virtual IEditor
{
public:
    explicit Editor(InterpMethod interpMethod, bool debug)
        : dataAccess{ fact::makeDataAccess() }
        , debug{ debug }
    {
        setInterpolationMethod(interpMethod);
    }
    
    virtual ~Editor() override { }
    
    // inherited via IEditor
    virtual auto loadImage(const QString& filepath) -> std::optional<QImage> override { return dataAccess->loadImage(filepath); }
    virtual void saveImage(const QString& filepath) const override                    { dataAccess->saveImage(filepath); }
    virtual auto getImage() const -> std::optional<QImage> override                   { return dataAccess->getImage(); }
    virtual void appendHistory(QImage image) override                                 { dataAccess->appendHistory(image); }
    virtual auto undo() -> std::optional<QImage> override                             { return dataAccess->undo(); }
    virtual auto redo() -> std::optional<QImage> override                             { return dataAccess->redo(); }
    
    virtual void setInterpolationMethod(InterpMethod method) override;
    virtual auto mergeImages(QImage lower, QImage upper, const QRect& upperRect, float upperAngle) -> QImage override;

private:
    std::unique_ptr<IDataAccess>     dataAccess;
    std::function<QRgb(const Cell&)> interpFunc;
    bool                             debug;

    static inline auto getDuration(const std::chrono::time_point<std::chrono::system_clock>& start,
                                   const std::chrono::time_point<std::chrono::system_clock>& end) -> QString
    {
        return QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + "ms";
    }
    
    static auto reverseRotate(const QPoint& p, const QRect& upperRect, float upperAngle) -> QPointF;
};
