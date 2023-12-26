#pragma once

#include <GL/glew.h>
#include "vertex_buffer.h"

class OpenGlVertexBuffer : public VertexBuffer {
public:
    OpenGlVertexBuffer(const void* data, uint32_t size_bytes, bool dynamic = false);
    OpenGlVertexBuffer(uint32_t max_size_bytes);
    ~OpenGlVertexBuffer();

public:
    void Bind() const override;
    void Unbind() const override;
    void UpdateVertices(const void* data, uint32_t offset, uint32_t size) override;

    uint32_t GetVerticesSizeBytes() const override;
    bool IsValid() const override;

private:
    GLuint renderer_id_;
    uint32_t buffer_size_;
};

