#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "vertex_buffer.h"

class IndexBuffer
{
public:
    IndexBuffer(const std::uint32_t* data,
        std::int32_t numIndices, bool bDynamic = false);
    IndexBuffer(std::int32_t totalNumIndices);
    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    std::int32_t GetNumIndices() const;

    void UpdateIndices(const std::uint32_t* data, std::int32_t offset, std::int32_t size);
    void UpdateIndices(const std::uint32_t* data, std::int32_t size);

    std::uint32_t GetOpenGlIdentifier() const;

private:
    std::uint32_t m_RendererId;
    std::int32_t m_NumIndices;
};


FORCE_INLINE std::int32_t IndexBuffer::GetNumIndices() const
{
    return m_NumIndices;
}

FORCE_INLINE void IndexBuffer::UpdateIndices(const std::uint32_t* data, std::int32_t size)
{
    UpdateIndices(data, 0, size);
}

FORCE_INLINE std::uint32_t IndexBuffer::GetOpenGlIdentifier() const
{
    return m_RendererId;
}
