#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "core.h"

class VertexBuffer {
public:
    VertexBuffer(const void* data, int size_bytes, bool dynamic = false);
    VertexBuffer(int max_size_bytes);
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;

    void UpdateVertices(const void* buffer, int offset, int size);

    void UpdateVertices(const void* buffer, int size);

    int GetVerticesSizeBytes() const;
    uint32_t GetOpenGlIdentifier() const;

private:
    uint32_t renderer_id_;
    int buffer_size_;
};

FORCE_INLINE void VertexBuffer::UpdateVertices(const void* buffer, int size) {
    UpdateVertices(buffer, 0, size);
}

FORCE_INLINE int VertexBuffer::GetVerticesSizeBytes() const {
    return buffer_size_;
}

FORCE_INLINE uint32_t VertexBuffer::GetOpenGlIdentifier() const {
    return renderer_id_;
}
