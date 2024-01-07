#pragma once

#include <GL/glew.h>
#include "vertex_buffer.h"

class OpenGlVertexBuffer : public VertexBuffer
{
public:
    OpenGlVertexBuffer(const void* data, std::int32_t size_bytes, bool dynamic = false);
    OpenGlVertexBuffer(std::int32_t max_size_bytes);
    ~OpenGlVertexBuffer();

public:
    void Bind() const override;
    void Unbind() const override;
    void UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size) override;

    std::int32_t GetVerticesSizeBytes() const override;
    bool IsValid() const override;

private:
    GLuint renderer_id_;
    std::int32_t buffer_size_;
};

