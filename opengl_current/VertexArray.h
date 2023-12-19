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
    
    void SetIndexBuffer(IndexBuffer&& ib);

    std::uint32_t GetNumIndices() const;

    VertexBuffer& GetVertexBufferAt(std::uint32_t index) { return _vertexBuffers[index]; }
    IndexBuffer& GetIndexBuffer()
    {
        return _indexBuffer;
    }

    template <typename T>
    void AddBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(VertexBuffer(data.data(), static_cast<std::uint32_t>(data.size_bytes())), attributes);
    }

    template <typename T>
    void AddDynamicBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(VertexBuffer(data.data(), data.size_bytes(), true), attributes);
    }

    void AddDynamicBuffer(std::uint32_t maxSize, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(VertexBuffer(maxSize), attributes);
    }

private:
    GLuint _rendererID;
    std::vector<VertexBuffer> _vertexBuffers;
    IndexBuffer _indexBuffer;

private:
    void AddBufferInternal(VertexBuffer&& vb, std::span<const VertexAttribute> attributes);
};

