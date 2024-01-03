#pragma once

#include "index_buffer.h"

#include <GL/glew.h>

class OpenGlIndexBuffer : public IIndexBuffer
{
public:
    OpenGlIndexBuffer();
    OpenGlIndexBuffer(const uint32_t* data, int32_t numIndices, bool bDynamic = false);
    OpenGlIndexBuffer(int32_t totalNumIndices);
    ~OpenGlIndexBuffer();

public:
    // Inherited via IndexBuffer
    void Bind() const override;
    void Unbind() const override;
    int32_t GetNumIndices() const override;
    void UpdateIndices(const uint32_t* data, const BufferSize& bufferSize) override;
    bool IsValid() const override;

private:
    uint32_t RendererId;
    int32_t NumIndices;
};

