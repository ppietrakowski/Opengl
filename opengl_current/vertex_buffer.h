#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

class IVertexBuffer
{
public:
    static std::shared_ptr<IVertexBuffer> Create(const void* data, uint32_t sizeBytes, bool dynamic = false);
    static std::shared_ptr<IVertexBuffer> CreateEmpty(uint32_t maxSizeBytes);
    virtual ~IVertexBuffer() = default;

public:

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual void UpdateVertices(const void* data, uint32_t offset, uint32_t size) = 0;
    virtual void UpdateVertices(const void* data, uint32_t size)
    {
        UpdateVertices(data, 0, size);
    }

    virtual uint32_t GetVerticesSizeBytes() const = 0;
    virtual bool IsValid() const = 0;
};