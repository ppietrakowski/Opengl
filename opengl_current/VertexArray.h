#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <memory>

enum class PrimitiveVertexType : std::uint8_t
{
    Int,
    UnsignedInt,
    Float,
    MaxPrimitiveVertexType
};

struct VertexAttribute
{
    std::uint8_t NumComponents : 5;
    PrimitiveVertexType Type : 3;
};

class VertexArray
{
public:
    VertexArray();
    VertexArray(VertexArray&& tempVertexArray) noexcept { *this = std::move(tempVertexArray); }

    VertexArray& operator=(VertexArray&& tempVertexArray) noexcept;
    ~VertexArray();

    void Bind() const;
    void Unbind() const;
    void AddBuffer(VertexBuffer&& vb, std::span<const VertexAttribute> attributes);
    void SetIndexBuffer(IndexBuffer&& ib);

    std::uint32_t GetNumIndices() const;

    VertexBuffer& GetVertexBufferAt(std::uint32_t index) { return _vertexBuffers[index]; }
    IndexBuffer& GetIndexBuffer()
    {
        return _indexBuffer;
    }

private:
    GLuint _rendererID;
    std::vector<VertexBuffer> _vertexBuffers;
    IndexBuffer _indexBuffer;
};

