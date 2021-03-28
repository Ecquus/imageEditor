#pragma once

#include "colordata.h"

#include <QImage>

class IMainWindow
{
public:
    virtual ~IMainWindow() { }

    virtual auto getImage() -> std::optional<QImage> = 0;
    virtual auto getColorData() -> ColorData = 0;
    virtual auto mergeImages(QImage lower, QImage upper, QRect upperRect, float upperAngle) -> QImage = 0;
};
