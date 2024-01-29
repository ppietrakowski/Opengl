#include "uniform_buffer.h"

#include <GL/glew.h>

UniformBuffer::UniformBuffer(int32_t maxSize) :
    m_MaxSize(maxSize)
{
    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);

    glBufferData(GL_UNIFORM_BUFFER, maxSize, nullptr, GL_DYNAMIC_DRAW);
    s_NumBytesAllocated += maxSize;
}

UniformBuffer::~UniformBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
    s_NumBytesAllocated -= m_MaxSize;
}

void UniformBuffer::UpdateBuffer(const void* data, int32_t sizeBytes)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeBytes, data);
}

void UniformBuffer::UpdateBuffer(const void* data, int32_t sizeBytes, int32_t offset)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeBytes, data);
}

void UniformBuffer::Bind(int32_t binding_id) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_id, m_RendererId);
}
