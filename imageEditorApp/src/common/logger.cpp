#include "logger.h"

#include <QDebug>

bool Logger::debugEnabled{ false };

auto Logger::getInstance() -> Logger&
{
    static Logger instance;
    return instance;
}

void Logger::setDebug(bool value)
{
    debugEnabled = value;
}

void Logger::debug(const QString& msg)
{
    if (debugEnabled)
        qDebug().noquote() << "Debug:  " << msg;
}

void Logger::warning(const QString& msg)
{
    if (debugEnabled)
        qDebug().noquote() << "Warning:" << msg;
}

void Logger::error(const QString& msg)
{
    qDebug().noquote() << "Error:  " << msg;    
}

void Logger::toView(const QString& msg)
{
    emit getInstance().logToView(msg);
}
