#include "vertex_buffer.h"
#include "error_macros.h"

#include "render_command.h"

#include <GL/glew.h>

VertexBuffer::VertexBuffer(const void* data, int size_bytes, bool dynamic) :
    buffer_size_{size_bytes}
{
    GLenum bufferUsage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ARRAY_BUFFER, size_bytes, data, bufferUsage);

    RenderCommand::NotifyVertexBufferCreated(size_bytes);
}

VertexBuffer::VertexBuffer(int max_size_bytes) :
    VertexBuffer{nullptr, max_size_bytes, true}
{
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &renderer_id_);
    RenderCommand::NotifyVertexBufferDestroyed(buffer_size_);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UpdateVertices(const void* buffer, int offset, int size)
{
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
}