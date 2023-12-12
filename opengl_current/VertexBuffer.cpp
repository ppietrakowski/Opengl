#include "VertexBuffer.h"

#include "ErrorMacros.h"
#include <GL/glew.h>

VertexBuffer::VertexBuffer() :
    _bufferSize{ 0 },
    _rendererID{ 0 }
{
}

VertexBuffer::VertexBuffer(const void* data, unsigned int size, bool dynamic) :
    _bufferSize{ size }
{
    GLenum bufferUsage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data, bufferUsage);
}

VertexBuffer::VertexBuffer(unsigned int size) :
    VertexBuffer{ nullptr, size, true }
{
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& buffer) noexcept
{
    std::swap(buffer._bufferSize, _bufferSize);
    std::swap(buffer._rendererID, _rendererID);

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

void VertexBuffer::UpdateVertices(const void* data, unsigned int offset, unsigned int size)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLintptr>(size), data);
}

unsigned int VertexBuffer::GetVerticesSizeBytes() const
{
    return _bufferSize;
}

void VertexBuffer::Release()
{
    glDeleteBuffers(1, &_rendererID);
    _rendererID = 0;
    _bufferSize = 0;
}
