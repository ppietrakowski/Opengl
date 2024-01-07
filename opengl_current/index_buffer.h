#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "vertex_buffer.h"

class IndexBuffer
{
public:
    static std::shared_ptr<IndexBuffer> Create(const std::uint32_t* data,
        std::int32_t num_indices, bool dynamic = false);

    static std::shared_ptr<IndexBuffer> CreateEmpty(std::int32_t total_num_indices);
    virtual ~IndexBuffer() = default;

public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual std::int32_t GetNumIndices() const = 0;

    virtual void UpdateIndices(const std::uint32_t* data, std::int32_t offset, std::int32_t size) = 0;
    void UpdateIndices(const std::uint32_t* data, std::int32_t size)
    {
        UpdateIndices(data, 0, size);
    }

    virtual bool IsValid() const = 0;
};
