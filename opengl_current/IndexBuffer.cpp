#include "IndexBuffer.h"
#include "ErrorMacros.h"

#include <GL/glew.h>

IndexBuffer::IndexBuffer() :
    _rendererID{ 0 },
    _numIndices{ 0 }
{
}

IndexBuffer::IndexBuffer(const std::uint32_t* data, std::uint32_t numIndices, bool dynamic) :
    _numIndices{ numIndices }
{
    GLenum bufferUsage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &_rendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(std::uint32_t), data, bufferUsage);
}

IndexBuffer::IndexBuffer(std::uint32_t totalNumIndices) :
    IndexBuffer{ nullptr, totalNumIndices, true }
{
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& buffer) noexcept
{
    _numIndices = buffer._numIndices;
    _rendererID = buffer._rendererID;
    buffer._numIndices = 0;
    buffer._rendererID = 0;
    return *this;
}

IndexBuffer::~IndexBuffer()
{
    Release();
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::UpdateIndices(const std::uint32_t* data, std::uint32_t offset, std::uint32_t numElements)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
    std::uint32_t sizeBytes = numElements * sizeof(std::uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeBytes, data);
}

void IndexBuffer::Release()
{
    glDeleteBuffers(1, &_rendererID);
    _rendererID = 0;
    _numIndices = 0;
}
