#include "VertexBuffer.h"

#include "ErrorMacros.h"
#include <GL/glew.h>

VertexBuffer::VertexBuffer() :
    _bufferSize{ 0 },
    _rendererID{ 0 }
{
}

VertexBuffer::VertexBuffer(const void* data, std::uint32_t sizeBytes, bool dynamic) :
    _bufferSize{ sizeBytes }
{
    GLenum bufferUsage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, bufferUsage);
}

VertexBuffer::VertexBuffer(std::uint32_t maxSizeBytes) :
    VertexBuffer{ nullptr, maxSizeBytes, true }
{
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& tempVertexBuffer) noexcept
{
    _bufferSize = tempVertexBuffer._bufferSize;
    _rendererID = tempVertexBuffer._rendererID;

    tempVertexBuffer._rendererID = 0;
    tempVertexBuffer._bufferSize = 0;
    return *this;
}

VertexBuffer::~VertexBuffer()
{
    Release();
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UpdateVertices(const void* data, std::uint32_t offset, std::uint32_t size)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLintptr>(size), data);
}

void VertexBuffer::Release()
{
    glDeleteBuffers(1, &_rendererID);
    _rendererID = 0;
    _bufferSize = 0;
}
