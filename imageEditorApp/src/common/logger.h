#pragma once

#include <QObject>
#include <QString>

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;

    static auto getInstance() -> Logger&;
    static void setDebug(bool enable);
    static void debug(const QString& msg);
    static void warning(const QString& msg);
    static void error(const QString& msg);
    static void toView(const QString& msg);

signals:
    void logToView(const QString& msg) const;

private:
    static bool debugEnabled;
    
    Logger() { }
};
