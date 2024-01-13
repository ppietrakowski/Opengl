#include "index_buffer.h"
#include "render_command.h"

#include <GL/glew.h>

#include "error_macros.h"
#include "renderer_api.h"


IndexBuffer::IndexBuffer(const std::uint32_t* data, std::int32_t numIndices, bool bDynamic) :
    m_NumIndices{numIndices}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(std::uint32_t), data, bufferUsage);

    RenderCommand::NotifyIndexBufferCreated(numIndices * sizeof(std::uint32_t));
}

IndexBuffer::IndexBuffer(std::int32_t totalNumIndices) :
    IndexBuffer{nullptr, totalNumIndices, true}
{
}

IndexBuffer::~IndexBuffer()
{
    RenderCommand::NotifyIndexBufferDestroyed(m_NumIndices * sizeof(std::uint32_t));
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

void IndexBuffer::UpdateIndices(const uint32_t* data, std::int32_t offset, std::int32_t size)
{
    ERR_FAIL_EXPECTED_TRUE_MSG(size <= m_NumIndices, "Size over declared is causing memory allocation -> may occur memory leak");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
    std::int32_t sizeBytes = size * sizeof(std::uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeBytes, data);
}
