#include "opengl_vertex_buffer.h"
#include "error_macros.h"

#include <GL/glew.h>

OpenGlVertexBuffer::OpenGlVertexBuffer(const void* data, int32_t sizeBytes, bool bDynamic) :
    m_BufferSize{sizeBytes}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, bufferUsage);
}

OpenGlVertexBuffer::OpenGlVertexBuffer(int32_t maxSizeBytes) :
    OpenGlVertexBuffer{nullptr, maxSizeBytes, true}
{
}

OpenGlVertexBuffer::~OpenGlVertexBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
    m_RendererId = 0;
    m_BufferSize = 0;
}

int32_t OpenGlVertexBuffer::GetVerticesSizeBytes() const
{
    return m_BufferSize;
}

bool OpenGlVertexBuffer::IsValid() const
{
    return m_RendererId != 0;
}

void OpenGlVertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
}

void OpenGlVertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGlVertexBuffer::UpdateVertices(const void* buffer, const BufferSize& bufferSize)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(bufferSize.Offset), static_cast<GLintptr>(bufferSize.Size), buffer);
}