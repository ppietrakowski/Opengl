#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

class VertexBuffer {
public:
    static std::shared_ptr<VertexBuffer> Create(const void* data, std::int32_t size_bytes, bool dynamic = false);
    static std::shared_ptr<VertexBuffer> CreateEmpty(std::int32_t max_size_bytes);
    virtual ~VertexBuffer() = default;

public:

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size) = 0;

    void UpdateVertices(const void* buffer, std::int32_t size) {
        UpdateVertices(buffer, 0, size);
    }

    virtual std::int32_t GetVerticesSizeBytes() const = 0;
    virtual bool IsValid() const = 0;
};