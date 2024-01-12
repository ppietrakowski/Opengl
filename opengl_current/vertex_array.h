#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

#include <cstdint>
#include <memory>

enum class PrimitiveVertexType : std::int8_t
{
    Int,
    UnsignedInt,
    Float,
    MaxPrimitiveVertexType
};

struct VertexAttribute
{
    std::int8_t NumComponents : 5;
    PrimitiveVertexType VertexType : 3;
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

    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

    std::int32_t GetNumIndices() const;

    std::shared_ptr<VertexBuffer> GetVertexBufferAt(std::int32_t index);
    std::shared_ptr<IndexBuffer> GetIndexBuffer();

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, AttributesView attributes);
    std::uint32_t GetOpenGlIdentifier() const;

private:
    std::uint32_t m_RendererId;
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;

private:
    void AddBufferInternal(const std::shared_ptr<VertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes);
};