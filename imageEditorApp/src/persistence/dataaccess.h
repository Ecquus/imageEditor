#pragma once

#include <deque>
#include <memory>
#include <QString>

#include <idataaccess.h>
#include <util.h>

class DataAccess : public virtual IDataAccess
{
public:
    virtual ~DataAccess() override { }

    // inherited via IDataAccess
    virtual auto loadImage(const QString& filepath) -> std::optional<QImage> override;
    virtual void saveImage(const QString& filepath) const override;
    virtual auto getImage() const -> std::optional<QImage> override;
    virtual void appendHistory(QImage) override;
    virtual auto undo() -> std::optional<QImage> override;
    virtual auto redo() -> std::optional<QImage> override;

private:
    using History = util::history<QImage, 10u>;
    std::unique_ptr<History> history{ nullptr};
};
