#pragma once

#include "VertexBuffer.h"
#include <cstdint>
#include <span>
#include <vector>

class IndexBuffer
{
public:
    IndexBuffer();
    IndexBuffer(const uint32_t* data, uint32_t num_indices, bool dynamic = false);
    IndexBuffer(uint32_t total_num_indices);
    IndexBuffer(IndexBuffer&& buffer) noexcept
    {
        *this = std::move(buffer);
    }

    IndexBuffer& operator=(IndexBuffer&& buffer) noexcept;

    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    uint32_t GetNumIndices() const { return num_indices_; }

    void UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t num_elements);
    void UpdateIndices(const uint32_t* data, uint32_t num_elements) { UpdateIndices(data, 0, num_elements); }

    bool IsValid() const { return renderer_id_ != 0; }
    GLuint GetRendererID() const { return renderer_id_; }

    void Release();

private:
    GLuint renderer_id_;
    uint32_t num_indices_;
};
