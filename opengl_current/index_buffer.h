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
        std::int32_t num_indices, bool dynamic = false);
    IndexBuffer(std::int32_t max_num_indices);
    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    std::int32_t GetNumIndices() const;

    void UpdateIndices(const std::uint32_t* data, std::int32_t offset, std::int32_t size);
    void UpdateIndices(const std::uint32_t* data, std::int32_t size);

    std::uint32_t GetOpenGlIdentifier() const;

private:
    std::uint32_t renderer_id_;
    std::int32_t num_indices_;
};


FORCE_INLINE std::int32_t IndexBuffer::GetNumIndices() const
{
    return num_indices_;
}

FORCE_INLINE void IndexBuffer::UpdateIndices(const std::uint32_t* data, std::int32_t size)
{
    UpdateIndices(data, 0, size);
}

FORCE_INLINE std::uint32_t IndexBuffer::GetOpenGlIdentifier() const
{
    return renderer_id_;
}
