#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <memory>

enum class EPrimitiveVertexType : short
{
    Int,
    UnsignedInt,
    Float,
    MaxPrimitiveVertexType
};

struct VertexAttribute
{
    unsigned int NumComponents;
    EPrimitiveVertexType Type;
};

class VertexArray
{
public:
    VertexArray();
    VertexArray(VertexArray&& array) noexcept { *this = std::move(array); }

    VertexArray& operator=(VertexArray&& array) noexcept;
    ~VertexArray();

    void Bind() const;
    void Unbind() const;
    void AddBuffer(VertexBuffer&& vb, std::span<const VertexAttribute> attributes);
    void SetIndexBuffer(IndexBuffer&& ib);

    unsigned int GetNumIndices() const;

    VertexBuffer& GetVertexBufferAt(unsigned int index) { return _vertexBuffers[index]; }
    IndexBuffer& GetIndexBuffer()
    {
        return _indexBuffer;
    }

private:
    GLuint _rendererID;
    std::vector<VertexBuffer> _vertexBuffers;
    IndexBuffer _indexBuffer;
};

