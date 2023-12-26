#pragma once

#include "index_buffer.h"

#include <GL/glew.h>

class OpenGlIndexBuffer : public IndexBuffer {
public:
    OpenGlIndexBuffer();
    OpenGlIndexBuffer(const uint32_t* data, uint32_t num_indices, bool dynamic = false);
    OpenGlIndexBuffer(uint32_t total_num_indices);
    ~OpenGlIndexBuffer();

public:
    // Inherited via IndexBuffer
    void Bind() const override;
    void Unbind() const override;
    uint32_t GetNumIndices() const override;
    void UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t num_elements) override;
    bool IsValid() const override;

private:
    uint32_t renderer_id_;
    uint32_t num_indices_;

};

