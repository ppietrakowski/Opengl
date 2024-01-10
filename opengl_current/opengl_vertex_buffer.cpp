#include "opengl_vertex_buffer.h"
#include "error_macros.h"

#include "vertex_buffer.h"

#include <GL/glew.h>

OpenGlVertexBuffer::OpenGlVertexBuffer(const void* data, std::int32_t sizeBytes, bool bDynamic) :
    m_BufferSize{sizeBytes}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, bufferUsage);
}

OpenGlVertexBuffer::OpenGlVertexBuffer(std::int32_t maxSizeBytes) :
    OpenGlVertexBuffer{nullptr, maxSizeBytes, true}
{
}

OpenGlVertexBuffer::~OpenGlVertexBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
    m_RendererId = 0;
    m_BufferSize = 0;
}

std::int32_t OpenGlVertexBuffer::GetVerticesSizeBytes() const
{
    return m_BufferSize;
}

void OpenGlVertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
}

void OpenGlVertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGlVertexBuffer::UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLintptr>(size), buffer);
}