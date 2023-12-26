#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

class VertexBuffer
{
public:
    VertexBuffer();
    VertexBuffer(const void* data, uint32_t size_bytes, bool dynamic = false);
    VertexBuffer(uint32_t max_size_bytes);
    VertexBuffer(VertexBuffer&& temp_vertex_buffer) noexcept
    {
        *this = std::move(temp_vertex_buffer);
    }

    VertexBuffer& operator=(VertexBuffer&& temp_vertex_buffer) noexcept;
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;
    void UpdateVertices(const void* data, uint32_t offset, uint32_t size);
    void UpdateVertices(const void* data, uint32_t size) { UpdateVertices(data, 0, size); }

    uint32_t GetVerticesSizeBytes() const { return buffer_size_; }

    bool IsValid() const { return renderer_id_ != 0; }
    void Release();

private:
    uint32_t renderer_id_;
    uint32_t buffer_size_;
};