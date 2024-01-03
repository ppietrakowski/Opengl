#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

struct BufferSize
{
    int32_t Size;
    int32_t Offset{0}; // Offset from buffer start

    BufferSize() = default;
    BufferSize(int32_t size) :
        Size{size}
    {
    }

    BufferSize(int32_t size, int32_t offset) :
        Size{size},
        Offset{offset}
    {
    }

    BufferSize(const BufferSize&) = default;
    BufferSize& operator=(const BufferSize&) = default;
};

class IVertexBuffer
{
public:
    static std::shared_ptr<IVertexBuffer> Create(const void* data, int32_t sizeBytes, bool dynamic = false);
    static std::shared_ptr<IVertexBuffer> CreateEmpty(int32_t maxSizeBytes);
    virtual ~IVertexBuffer() = default;

public:

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    
    virtual void UpdateVertices(const void* buffer, BufferSize bufferSize) = 0;

    virtual int32_t GetVerticesSizeBytes() const = 0;
    virtual bool IsValid() const = 0;
};