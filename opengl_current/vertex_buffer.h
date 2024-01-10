#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

class VertexBuffer
{
public:
    static std::shared_ptr<VertexBuffer> Create(const void* data, std::int32_t sizeBytes, bool bDynamic = false);
    static std::shared_ptr<VertexBuffer> CreateEmpty(std::int32_t maxSizeBytes);
    virtual ~VertexBuffer() = default;

public:

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size) = 0;

    void UpdateVertices(const void* buffer, std::int32_t size)
    {
        UpdateVertices(buffer, 0, size);
    }

    virtual std::int32_t GetVerticesSizeBytes() const = 0;
};