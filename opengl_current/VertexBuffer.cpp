#include "VertexBuffer.hpp"
#include "ErrorMacros.hpp"

#include "RenderCommand.hpp"

#include <GL/glew.h>

VertexBuffer::VertexBuffer(const void* data, int sizeBytes, bool bDynamic) :
    m_BufferSize{sizeBytes}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, bufferUsage);

    s_NumVertexBufferMemoryAllocated += sizeBytes;
}

VertexBuffer::VertexBuffer(int maxSizeBytes) :
    VertexBuffer{nullptr, maxSizeBytes, true}
{
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
    s_NumVertexBufferMemoryAllocated -= m_BufferSize;
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UpdateVertices(const void* buffer, int offset, int size)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
}