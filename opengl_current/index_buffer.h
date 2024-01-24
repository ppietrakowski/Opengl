#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "vertex_buffer.h"

class IndexBuffer
{
public:
    IndexBuffer(const uint32_t* data,
        int numIndices, bool bDynamic = false);
    IndexBuffer(int maxNumIndices);
    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    int GetNumIndices() const;

    void UpdateIndices(const uint32_t* data, int offset, int size);
    void UpdateIndices(const uint32_t* data, int size);

    uint32_t GetOpenGlIdentifier() const;

private:
    uint32_t m_RendererId;
    int m_NumIndices;
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
