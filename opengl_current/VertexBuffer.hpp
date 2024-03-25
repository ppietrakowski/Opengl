#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "Core.hpp"

class VertexBuffer
{
public:
    VertexBuffer(const void* data, int sizeBytes, bool bDynamic = false);
    VertexBuffer(int maxSizeBytes);
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;

    void UpdateVertices(const void* buffer, int offset, int size);
    void UpdateVertices(const void* buffer, int size);

    template <typename T>
    void UpdateVertex(const T& vertex, int startIndex = 0)
    {
        UpdateVertices(&vertex, sizeof(T), startIndex * sizeof(T));
    }

    template <typename Container>
    void Update(const Container& vertices, int startIndex = 0)
    {
        UpdateVertices(vertices.data(), GetContainerSizeInt(vertices) * sizeof(Container::value_type), startIndex * sizeof(Container::value_type));
    }

    int GetVerticesSizeBytes() const;
    uint32_t GetOpenGlIdentifier() const;
    
    static inline size_t s_NumVertexBufferMemoryAllocated = 0;

private:
    uint32_t m_RendererId;
    int m_BufferSize;
};

FORCE_INLINE void VertexBuffer::UpdateVertices(const void* buffer, int size)
{
    UpdateVertices(buffer, 0, size);
}

FORCE_INLINE int VertexBuffer::GetVerticesSizeBytes() const
{
    return m_BufferSize;
}

FORCE_INLINE uint32_t VertexBuffer::GetOpenGlIdentifier() const
{
    return m_RendererId;
}
