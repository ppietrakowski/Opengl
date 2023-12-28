#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

#include <memory>

enum class PrimitiveVertexType : int8_t
{
    kInt,
    kUnsignedInt,
    kFloat,
    kMaxPrimitiveVertexType
};

struct VertexAttribute
{
    int8_t NumComponents : 5;
    PrimitiveVertexType VertexType : 3;
};

class IVertexArray
{
public:
    static std::shared_ptr<IVertexArray> Create();
    virtual ~IVertexArray() = default;

public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetIndexBuffer(const std::shared_ptr<IIndexBuffer>& indexBuffer) = 0;

    virtual int32_t GetNumIndices() const = 0;

    virtual std::shared_ptr<IVertexBuffer> GetVertexBufferAt(int32_t index) = 0;
    virtual std::shared_ptr<IIndexBuffer> GetIndexBuffer() = 0;

    template <typename T>
    void AddBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(IVertexBuffer::Create(data.data(), static_cast<int32_t>(data.size_bytes())), attributes);
    }

    template <typename T>
    void AddDynamicBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(IVertexBuffer::Create(data.data(), static_cast<int32_t>(data.size_bytes()), true), attributes);
    }

    void AddDynamicBuffer(int32_t maxSize, std::span<const VertexAttribute> attributes)
    {
        AddBufferInternal(IVertexBuffer::CreateEmpty(maxSize), attributes);
    }

protected:
    virtual void AddBufferInternal(const std::shared_ptr<IVertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes) = 0;
};
