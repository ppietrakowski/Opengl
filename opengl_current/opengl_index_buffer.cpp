#include "opengl_index_buffer.h"
#include "error_macros.h"

OpenGlIndexBuffer::OpenGlIndexBuffer() :
    RendererId{0},
    NumIndices{0}
{
}

OpenGlIndexBuffer::OpenGlIndexBuffer(const uint32_t* data, int32_t numIndices, bool bDynamic) :
    NumIndices{numIndices}
{
    GLenum bufferUsage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &RendererId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), data, bufferUsage);
}

OpenGlIndexBuffer::OpenGlIndexBuffer(int32_t totalNumIndices) :
    OpenGlIndexBuffer{nullptr, totalNumIndices, true}
{
}

OpenGlIndexBuffer::~OpenGlIndexBuffer()
{
    glDeleteBuffers(1, &RendererId);
    RendererId = 0;
    NumIndices = 0;
}

int32_t OpenGlIndexBuffer::GetNumIndices() const
{
    return NumIndices;
}

void OpenGlIndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererId);
}

void OpenGlIndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGlIndexBuffer::UpdateIndices(const uint32_t* data, const BufferSize& bufferSize)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RendererId);
    int32_t sizeBytes = bufferSize.Size * sizeof(uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, bufferSize.Offset, sizeBytes, data);
}

bool OpenGlIndexBuffer::IsValid() const
{
    return RendererId != 0;
}
