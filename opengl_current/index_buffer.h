#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "vertex_buffer.h"

class IndexBuffer
{
public:
    IndexBuffer(std::span<uint32_t> data, bool bDynamic = false);
    IndexBuffer(int32_t maxNumIndices);
    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    int32_t GetNumIndices() const;

    void UpdateIndices(const uint32_t* data, int32_t offset, int32_t size);
    void UpdateIndices(const uint32_t* data, int32_t size);

    uint32_t GetOpenGlIdentifier() const;

    static inline size_t s_IndexBufferMemoryAllocation{0};

private:
    uint32_t m_RendererId;
    int32_t m_NumIndices;

private:
    void GenerateRendererId(const uint32_t *indices, bool bDynamic);
};


FORCE_INLINE int32_t IndexBuffer::GetNumIndices() const
{
    return m_NumIndices;
}

FORCE_INLINE void IndexBuffer::UpdateIndices(const uint32_t* data, int32_t size)
{
    UpdateIndices(data, 0, size);
}

FORCE_INLINE uint32_t IndexBuffer::GetOpenGlIdentifier() const
{
    return m_RendererId;
}
