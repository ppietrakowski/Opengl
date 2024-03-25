#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "VertexBuffer.hpp"

class IndexBuffer
{
public:
    IndexBuffer(std::span<uint32_t> data, bool bDynamic = false);
    IndexBuffer(int maxNumIndices);
    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    int GetNumIndices() const;

    void UpdateIndices(const uint32_t* data, int offset, int size);
    void UpdateIndices(const uint32_t* data, int size);

    uint32_t GetOpenGlIdentifier() const;

    static inline int64_t s_IndexBufferMemoryAllocation{0};

private:
    uint32_t m_RendererId;
    int m_NumIndices;

private:
    void GenerateRendererId(const uint32_t *indices, bool bDynamic);
};


FORCE_INLINE int IndexBuffer::GetNumIndices() const
{
    return m_NumIndices;
}

FORCE_INLINE void IndexBuffer::UpdateIndices(const uint32_t* data, int size)
{
    UpdateIndices(data, 0, size);
}

FORCE_INLINE uint32_t IndexBuffer::GetOpenGlIdentifier() const
{
    return m_RendererId;
}
