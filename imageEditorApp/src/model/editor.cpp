#include "editor.h"
#include <logger.h>
#include <util.h>

#include <cmath>
#include <cstdint>
#include <QPoint>
#include <QVector3D>
#include <QMatrix4x4>
#include <QDebug>
#include <qimage.h>
#include <qpoint.h>

// macros to conditionally compile timers for benchmarks
#ifndef NDEBUG
  #define START_TIMER const auto __debug__timerStart = std::chrono::system_clock::now();
#else
  #define START_TIMER
#endif

#ifndef NDEBUG
  #define STOP_TIMER \
    const auto __debug__timerEnd = std::chrono::system_clock::now(); \
    const auto __debug__timerDur = QString::number( \
        std::chrono::duration_cast<std::chrono::milliseconds>(__debug__timerEnd - __debug__timerStart).count()); \
    Logger::debug(QString{ "Function '" } + __func__ + "' ran for " + __debug__timerDur + " ms");
#else
  #define STOP_TIMER
#endif

void Editor::setInterpolationMethod(InterpMethod method)
{
    switch (method)
    {
        case InterpMethod::NEAREST:  interpFunc = Interpolator::nearest;  break;
        case InterpMethod::BILINEAR: interpFunc = Interpolator::bilinear; break;
        default:
            Logger::warning(QString{ "Invalid method passed to " } + __func__ + "!");
            interpFunc = Interpolator::nearest;
    }
}

auto Editor::mergeImages(QImage lower, QImage upper, const QRect& upperRect, float upperAngle) -> QImage
{
    const auto start  = std::chrono::system_clock::now();
    const auto offset = QPointF{ upperRect.topLeft() };

    // QImage would perform a copy of the image if bits() would be called, so we
    // need to query constBits() and cast away its constness to avoid this copy
    auto data = reinterpret_cast<QRgb*>(const_cast<uchar*>(lower.constBits()));
    
    for (int i = 0; i < lower.height(); ++i)
    {
        for (int j = 0; j < lower.width(); ++j)
        {
            const auto pixel = data + i * lower.width() + j;
            const auto revp  = reverseRotate({ j, i }, upperRect, upperAngle);
            if (upperRect.contains(util::roundPoint(revp)))
                *pixel = interpFunc(Cell{ revp - offset, upper, *pixel });
        }
    }
    
    const auto end = std::chrono::system_clock::now();
    Logger::toView("Action executed for " + getDuration(start, end));

    return lower;
}

// rotate p by -upperAngle around upperRect's center
auto Editor::reverseRotate(const QPoint& p, const QRect& upperRect, float upperAngle) -> QPointF
{
    auto q = QPointF{ p };

    q -= upperRect.center();

    auto m = QMatrix4x4{};
    m.rotate(-upperAngle, 0.0f, 0.0f, 1.0f);
    q = m * q;

    q += upperRect.center();

    return q;
}