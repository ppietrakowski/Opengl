#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "vertex_buffer.h"

class IIndexBuffer
{
public:
    static std::shared_ptr<IIndexBuffer> Create(const uint32_t* data,
        int32_t numIndices, bool bDynamic = false);

    static std::shared_ptr<IIndexBuffer> CreateEmpty(int32_t totalNumIndices);
    virtual ~IIndexBuffer() = default;

public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual int32_t GetNumIndices() const = 0;

    virtual void UpdateIndices(const uint32_t* data, const BufferSize& bufferSize) = 0;
    virtual bool IsValid() const = 0;
};
