#include "opengl_vertex_buffer.h"
#include "error_macros.h"

#include "vertex_buffer.h"

#include <GL/glew.h>

OpenGlVertexBuffer::OpenGlVertexBuffer(const void* data, int32_t sizeBytes, bool bDynamic) :
    VboBufferSize{sizeBytes}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &RendererId);
    glBindBuffer(GL_ARRAY_BUFFER, RendererId);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, bufferUsage);
}

OpenGlVertexBuffer::OpenGlVertexBuffer(int32_t maxSizeBytes) :
    OpenGlVertexBuffer{nullptr, maxSizeBytes, true}
{
}

OpenGlVertexBuffer::~OpenGlVertexBuffer()
{
    glDeleteBuffers(1, &RendererId);
    RendererId = 0;
    VboBufferSize = 0;
}

int32_t OpenGlVertexBuffer::GetVerticesSizeBytes() const
{
    return VboBufferSize;
}

bool OpenGlVertexBuffer::IsValid() const
{
    return RendererId != 0;
}

void OpenGlVertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, RendererId);
}

void OpenGlVertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGlVertexBuffer::UpdateVertices(const void* buffer, BufferSize bufferSize)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ARRAY_BUFFER, RendererId);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(bufferSize.Offset), static_cast<GLintptr>(bufferSize.Size), buffer);
}