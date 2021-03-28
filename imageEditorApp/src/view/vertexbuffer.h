#pragma once

#include <array>
#include <memory>
#include <QOpenGLBuffer>
#include <QPointF>

class VertexBuffer : public QOpenGLBuffer
{
public:
    static std::shared_ptr<VertexBuffer>
    make(const std::array<QPointF, 4>& texcoords = { QPointF{ 0.0f, 0.0f },
                                                     QPointF{ 1.0f, 0.0f },
                                                     QPointF{ 1.0f, 1.0f },
                                                     QPointF{ 0.0f, 1.0f } });
    void bindVbo();

private:
    VertexBuffer() : QOpenGLBuffer{} { }
    
    void allocVbo(const std::array<QPointF, 4>& texcoords);
};
