#pragma once

#include <GL/glew.h>
#include "vertex_buffer.h"

class OpenGlVertexBuffer : public IVertexBuffer
{
public:
    OpenGlVertexBuffer(const void* data, int32_t sizeBytes, bool bDynamic = false);
    OpenGlVertexBuffer(int32_t maxSizeBytes);
    ~OpenGlVertexBuffer();

public:
    void Bind() const override;
    void Unbind() const override;
    void UpdateVertices(const void* buffer, BufferSize bufferSize) override;

    int32_t GetVerticesSizeBytes() const override;
    bool IsValid() const override;

private:
    GLuint RendererId;
    int32_t VboBufferSize;
};

