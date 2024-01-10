#pragma once

#include <GL/glew.h>
#include "vertex_buffer.h"

class OpenGlVertexBuffer : public VertexBuffer
{
public:
    OpenGlVertexBuffer(const void* data, std::int32_t sizeBytes, bool bDynamic = false);
    OpenGlVertexBuffer(std::int32_t maxSizeBytes);
    ~OpenGlVertexBuffer();

public:
    void Bind() const override;
    void Unbind() const override;
    void UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size) override;

    std::int32_t GetVerticesSizeBytes() const override;

private:
    GLuint m_RendererId;
    std::int32_t m_BufferSize;
};

