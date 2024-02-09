#include <GL/glew.h>

#include "IndexBuffer.hpp"
#include "RendererApi.hpp"
#include "ErrorMacros.hpp"
#include "RenderCommand.hpp"

IndexBuffer::IndexBuffer(std::span<uint32_t> data, bool bDynamic) :
    m_NumIndices{static_cast<int32_t>(data.size())}
{
    GenerateRendererId(data.data(), bDynamic);
}

IndexBuffer::IndexBuffer(int32_t maxNumIndices) :
    m_NumIndices(maxNumIndices)
{
    GenerateRendererId(nullptr, true);
}

IndexBuffer::~IndexBuffer()
{
    s_IndexBufferMemoryAllocation -= m_NumIndices * sizeof(uint32_t);
    glDeleteBuffers(1, &m_RendererId);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::UpdateIndices(const uint32_t* data, int32_t offset, int32_t size)
{
    ERR_FAIL_EXPECTED_TRUE_MSG(size <= m_NumIndices, "Size over declared is causing memory allocation -> may occur memory leak");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
    int32_t sizeBytes = size * sizeof(uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeBytes, data);
}

void IndexBuffer::GenerateRendererId(const uint32_t* indices, bool bDynamic)
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_NumIndices * sizeof(uint32_t), indices, bufferUsage);

    s_IndexBufferMemoryAllocation += m_NumIndices * sizeof(uint32_t);
}