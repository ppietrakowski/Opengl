#pragma once

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

#include <cstdint>
#include <memory>

enum class PrimitiveVertexType : int8_t
{
    Int,
    UnsignedInt,
    Float,
    MaxPrimitiveVertexType
};

struct VertexAttribute
{
    int8_t NumComponents : 5;
    PrimitiveVertexType VertexType : 3;
};

using AttributesView = std::span<const VertexAttribute>;

class VertexArray
{
public:
    VertexArray();
    VertexArray(VertexArray&& tempArray) noexcept;
    VertexArray& operator=(VertexArray&& tempArray) noexcept;

    ~VertexArray();

public:
    void Bind() const;
    void Unbind() const;

    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

    int GetNumIndices() const;

    std::shared_ptr<VertexBuffer> GetVertexBufferAt(int index);
    std::shared_ptr<IndexBuffer> GetIndexBuffer();

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, AttributesView attributes);
    uint32_t GetOpenGlIdentifier() const;

    AttributesView GetAttributes() const;

private:
    uint32_t m_RendererId;
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    std::vector<VertexAttribute> m_Attributes;
};

FORCE_INLINE AttributesView VertexArray::GetAttributes() const
{
    return m_Attributes;
}