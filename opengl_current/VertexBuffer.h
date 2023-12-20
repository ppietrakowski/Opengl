#pragma once

#include <gl/glew.h>

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "Core.h"

class VertexBuffer
{
public:
    VertexBuffer();
    VertexBuffer(const void* data, std::uint32_t size_bytes, bool dynamic = false);
    VertexBuffer(std::uint32_t max_size_bytes);
    VertexBuffer(VertexBuffer&& temp_vertex_buffer) noexcept
    {
        *this = std::move(temp_vertex_buffer);
    }

    VertexBuffer& operator=(VertexBuffer&& temp_vertex_buffer) noexcept;
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;
    void UpdateVertices(const void* data, std::uint32_t offset, std::uint32_t size);
    void UpdateVertices(const void* data, std::uint32_t size) { UpdateVertices(data, 0, size); }

    std::uint32_t GetVerticesSizeBytes() const { return buffer_size_; }

    bool IsValid() const { return renderer_id_ != 0; }
    GLuint GetRendererID() const { return renderer_id_; }

    void Release();

private:
    GLuint renderer_id_;
    std::uint32_t buffer_size_;
};