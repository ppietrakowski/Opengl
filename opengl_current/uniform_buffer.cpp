#include "uniform_buffer.h"

#include <GL/glew.h>

UniformBuffer::UniformBuffer(int maxSize) :
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

void UniformBuffer::UpdateBuffer(const void* data, int sizeBytes)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeBytes, data);
}

void UniformBuffer::UpdateBuffer(const void* data, int sizeBytes, int offset)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererId);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeBytes, data);
}

void UniformBuffer::Bind(int binding_id) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_id, m_RendererId);
}
