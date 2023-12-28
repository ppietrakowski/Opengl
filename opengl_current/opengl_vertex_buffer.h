#pragma once

#include <GL/glew.h>
#include "vertex_buffer.h"

class OpenGlVertexBuffer : public IVertexBuffer
{
public:
    OpenGlVertexBuffer(const void* data, uint32_t sizeBytes, bool bDynamic = false);
    OpenGlVertexBuffer(uint32_t maxSizeBytes);
    ~OpenGlVertexBuffer();

public:
    void Bind() const override;
    void Unbind() const override;
    void UpdateVertices(const void* data, uint32_t offset, uint32_t size) override;

    uint32_t GetVerticesSizeBytes() const override;
    bool IsValid() const override;

private:
    GLuint m_RendererId;
    uint32_t m_BufferSize;
};

