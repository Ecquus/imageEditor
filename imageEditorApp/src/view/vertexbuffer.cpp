#include <util.h>
#include "logger.h"
#include "openglexception.h"
#include "vertexbuffer.h"

#include <QDebug>

using namespace util::types;

void VertexBuffer::bindVbo()
{
    assert(isCreated() && "VBO is null or in an invalid state during binding!");

    if (!bind())
        throw OpenGLException{ "Failed to bind VBO! "
                               "(This OpenGL implementation does not support this type of buffer, or "
                               "the current context differs from the one that created this buffer)" };
}

void VertexBuffer::allocVbo(const std::array<QPointF, 4>& texcoords)
{
    bindVbo();

    const auto coords = std::array<float, 16> {
       -1.0f, -1.0f, toFloat(texcoords[0].x()), toFloat(texcoords[0].y()),
        1.0f, -1.0f, toFloat(texcoords[1].x()), toFloat(texcoords[1].y()),
        1.0f,  1.0f, toFloat(texcoords[2].x()), toFloat(texcoords[2].y()),
       -1.0f,  1.0f, toFloat(texcoords[3].x()), toFloat(texcoords[3].y())
    };

    allocate(coords.data(), coords.size() * sizeof(coords[0]));
}

std::shared_ptr<VertexBuffer> VertexBuffer::make(const std::array<QPointF, 4>& texcoords)
{
    auto vbo = new VertexBuffer;
    
    if (!vbo->create())
        throw OpenGLException{ "Failed to create VBO! "
                               "(This OpenGL implementation does not support buffers, or "
                               "there is no current QOpenGLContext)" };

    vbo->allocVbo(texcoords);

    return std::shared_ptr<VertexBuffer>{ vbo };
}
