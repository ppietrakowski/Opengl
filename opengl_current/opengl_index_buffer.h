#pragma once

#include "index_buffer.h"

#include <GL/glew.h>

class OpenGlIndexBuffer : public IndexBuffer
{
public:
    OpenGlIndexBuffer();
    OpenGlIndexBuffer(const std::uint32_t* data, std::int32_t numIndices, bool bDynamic = false);
    OpenGlIndexBuffer(std::int32_t totalNumIndices);
    ~OpenGlIndexBuffer();

public:
    // Inherited via IndexBuffer
    void Bind() const override;
    void Unbind() const override;
    std::int32_t GetNumIndices() const override;
    void UpdateIndices(const std::uint32_t* data, std::int32_t offset, std::int32_t size) override;

private:
    GLuint m_RendererId;
    std::int32_t m_NumIndices;
};

