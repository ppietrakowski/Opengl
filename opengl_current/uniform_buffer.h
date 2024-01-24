#pragma once

#include <cstdint>

class UniformBuffer {
public:
    UniformBuffer(int maxSize);
    ~UniformBuffer();

    void UpdateBuffer(const void* data, int sizeBytes);
    void UpdateBuffer(const void* data, int sizeBytes, int offset);

    void Bind(int binding_id) const;

    static inline size_t s_NumBytesAllocated = 0;

private:
    uint32_t m_RendererId;
    int m_MaxSize;
};
