#pragma once

#include <stdexcept>
#include <QString>

class OpenGLException : public std::runtime_error
{
public:
    explicit OpenGLException(const char* what) : std::runtime_error{ what } { }
    explicit OpenGLException(const QString& what) : std::runtime_error{ what.toUtf8().constData() } { }
};

