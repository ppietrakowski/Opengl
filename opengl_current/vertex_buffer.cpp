#include "vertex_buffer.h"
#include "error_macros.h"

#include <GL/glew.h>

VertexBuffer::VertexBuffer(const void* data, std::int32_t sizeBytes, bool bDynamic) :
    m_BufferSize{sizeBytes}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, bufferUsage);
}

VertexBuffer::VertexBuffer(std::int32_t maxSizeBytes) :
    VertexBuffer{nullptr, maxSizeBytes, true}
{
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLintptr>(size), buffer);
}