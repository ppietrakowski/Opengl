#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

class VertexBuffer
{
public:
    VertexBuffer(const void* data, std::int32_t sizeBytes, bool bDynamic = false);
    VertexBuffer(std::int32_t maxSizeBytes);
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;

    void UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size);

    void UpdateVertices(const void* buffer, std::int32_t size);

    std::int32_t GetVerticesSizeBytes() const;
    std::uint32_t GetOpenGlIdentifier() const;

private:
    std::uint32_t m_RendererId;
    std::int32_t m_BufferSize;
};

FORCE_INLINE void VertexBuffer::UpdateVertices(const void* buffer, std::int32_t size)
{
    UpdateVertices(buffer, 0, size);
}

FORCE_INLINE std::int32_t VertexBuffer::GetVerticesSizeBytes() const
{
    return m_BufferSize;
}

FORCE_INLINE std::uint32_t VertexBuffer::GetOpenGlIdentifier() const
{
    return m_RendererId;
}
