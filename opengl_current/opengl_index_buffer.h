#pragma once

#include "index_buffer.h"

#include <GL/glew.h>

class OpenGlIndexBuffer : public IndexBuffer {
public:
    OpenGlIndexBuffer();
    OpenGlIndexBuffer(const std::uint32_t* data, std::int32_t num_indices, bool dynamic = false);
    OpenGlIndexBuffer(std::int32_t total_num_indices);
    ~OpenGlIndexBuffer();

public:
    // Inherited via IndexBuffer
    void Bind() const override;
    void Unbind() const override;
    std::int32_t GetNumIndices() const override;
    void UpdateIndices(const std::uint32_t* data, std::int32_t offset, std::int32_t size) override;
    bool IsValid() const override;

private:
    GLuint renderer_id_;
    std::int32_t num_indices_;
};

