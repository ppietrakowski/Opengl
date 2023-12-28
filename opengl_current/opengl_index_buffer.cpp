#include "opengl_index_buffer.h"
#include "error_macros.h"

OpenGlIndexBuffer::OpenGlIndexBuffer() :
    m_RendererId{0},
    m_NumIndices{0}
{
}

OpenGlIndexBuffer::OpenGlIndexBuffer(const uint32_t* data, uint32_t numIndices, bool bDynamic) :
    m_NumIndices{numIndices}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &m_RendererId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), data, bufferUsage);
}

OpenGlIndexBuffer::OpenGlIndexBuffer(uint32_t totalNumIndices) :
    OpenGlIndexBuffer{nullptr, totalNumIndices, true}
{
}

OpenGlIndexBuffer::~OpenGlIndexBuffer()
{
    glDeleteBuffers(1, &m_RendererId);
    m_RendererId = 0;
    m_NumIndices = 0;
}

uint32_t OpenGlIndexBuffer::GetNumIndices() const
{
    return m_NumIndices;
}

void OpenGlIndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
}

void OpenGlIndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGlIndexBuffer::UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t numIndices)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
    uint32_t sizeBytes = numIndices * sizeof(uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeBytes, data);
}

bool OpenGlIndexBuffer::IsValid() const
{
    return m_RendererId != 0;
}
