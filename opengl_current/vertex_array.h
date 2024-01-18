#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

#include <cstdint>
#include <memory>

enum class PrimitiveVertexType : std::int8_t
{
    kInt,
    kUnsignedInt,
    kFloat,
    kMaxPrimitiveVertexType
};

struct VertexAttribute
{
    std::int8_t num_components : 5;
    PrimitiveVertexType vertex_type : 3;
};

using AttributesView = std::span<const VertexAttribute>;

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

public:
    void Bind() const;
    void Unbind() const;

    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer);

    std::int32_t GetNumIndices() const;

    std::shared_ptr<VertexBuffer> GetVertexBufferAt(std::int32_t index);
    std::shared_ptr<IndexBuffer> GetIndexBuffer();

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer, AttributesView attributes);
    std::uint32_t GetOpenGlIdentifier() const;

    AttributesView GetAttributes() const
    {
        return attributes_;
    }

private:
    std::uint32_t renderer_id_;
    std::vector<std::shared_ptr<VertexBuffer>> vertex_buffers_;
    std::shared_ptr<IndexBuffer> index_buffer_;
    std::vector<VertexAttribute> attributes_;
};