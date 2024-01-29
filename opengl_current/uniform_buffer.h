#pragma once

#include <cstdint>

class UniformBuffer {
public:
    UniformBuffer(int32_t maxSize);
    ~UniformBuffer();

    void UpdateBuffer(const void* data, int32_t sizeBytes);
    void UpdateBuffer(const void* data, int32_t sizeBytes, int32_t offset);

    void Bind(int32_t bindingId) const;

    static inline size_t s_NumBytesAllocated = 0;

private:
    uint32_t m_RendererId;
    int32_t m_MaxSize;
};
