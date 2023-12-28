#pragma once

#include "index_buffer.h"

#include <GL/glew.h>

class OpenGlIndexBuffer : public IIndexBuffer
{
public:
    OpenGlIndexBuffer();
    OpenGlIndexBuffer(const uint32_t* data, uint32_t numIndices, bool bDynamic = false);
    OpenGlIndexBuffer(uint32_t totalNumIndices);
    ~OpenGlIndexBuffer();

public:
    // Inherited via IndexBuffer
    void Bind() const override;
    void Unbind() const override;
    uint32_t GetNumIndices() const override;
    void UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t numElements) override;
    bool IsValid() const override;

private:
    uint32_t m_RendererId;
    uint32_t m_NumIndices;

};

