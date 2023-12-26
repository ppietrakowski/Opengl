#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

#include <memory>

enum class PrimitiveVertexType : uint8_t
{
    kInt,
    kUnsignedInt,
    kFloat,
    kMaxPrimitiveVertexType
};

struct VertexAttribute
{
    uint8_t num_components : 5;
    PrimitiveVertexType vertex_type : 3;
};

class VertexArray
{
public:
    VertexArray();
    VertexArray(VertexArray&& temp_vertex_array) noexcept { *this = std::move(temp_vertex_array); }

    VertexArray& operator=(VertexArray&& temp_vertex_array) noexcept;
    ~VertexArray();

    void Bind() const;
    void Unbind() const;
    
    void SetIndexBuffer(IndexBuffer&& index_buffer);

    uint32_t GetNumIndices() const;

    VertexBuffer& GetVertexBufferAt(uint32_t index) { return vertex_buffers_[index]; }
    IndexBuffer& GetIndexBuffer()
    {
        return index_buffer_;
    }

    template <typename T>
    void AddBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(VertexBuffer(data.data(), static_cast<uint32_t>(data.size_bytes())), attributes);
    }

    template <typename T>
    void AddDynamicBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(VertexBuffer(data.data(), data.size_bytes(), true), attributes);
    }

    void AddDynamicBuffer(uint32_t max_size, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(VertexBuffer(max_size), attributes);
    }

private:
    uint32_t renderer_id_;
    std::vector<VertexBuffer> vertex_buffers_;
    IndexBuffer index_buffer_;

private:
    void AddBufferInternal(VertexBuffer&& vertex_buffer, std::span<const VertexAttribute> attributes);
};
