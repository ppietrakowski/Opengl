#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

#include <memory>

enum class PrimitiveVertexType : std::int8_t {
    kInt,
    kUnsignedInt,
    kFloat,
    kMaxPrimitiveVertexType
};

struct VertexAttribute {
    std::int8_t num_components : 5;
    PrimitiveVertexType vertex_type : 3;
};

class VertexArray {
public:
    static std::shared_ptr<VertexArray> Create();
    virtual ~VertexArray() = default;

public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) = 0;

    virtual std::int32_t GetNumIndices() const = 0;

    virtual std::shared_ptr<VertexBuffer> GetVertexBufferAt(std::int32_t index) = 0;
    virtual std::shared_ptr<IndexBuffer> GetIndexBuffer() = 0;

    template <typename T>
    void AddBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes) {
        AddBufferInternal(VertexBuffer::Create(data.data(), static_cast<std::int32_t>(data.size_bytes())), attributes);
    }

    template <typename T>
    void AddDynamicBuffer(std::span<const T> data, std::span<const VertexAttribute> attributes) {
        AddBufferInternal(VertexBuffer::Create(data.data(), static_cast<std::int32_t>(data.size_bytes()), true), attributes);
    }

    void AddDynamicBuffer(std::int32_t max_size, std::span<const VertexAttribute> attributes) {
        AddBufferInternal(VertexBuffer::CreateEmpty(max_size), attributes);
    }

protected:
    virtual void AddBufferInternal(const std::shared_ptr<VertexBuffer>& vertex_buffer, std::span<const VertexAttribute> attributes) = 0;
};
