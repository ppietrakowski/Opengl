#pragma once

#include <cstdint>
#include "Core.hpp"

class UniformBuffer {
public:
    UniformBuffer(int maxSize);
    ~UniformBuffer();

    void UpdateBuffer(const void* data, int sizeBytes);
    void UpdateBuffer(const void* data, int sizeBytes, int offset);

    template <typename T>
    void UpdateElement(const T& value, int startIndex)
    {
        UpdateBuffer(&value, sizeof(value), startIndex * sizeof(T));
    }

    template <typename T>
    void UpdateRange(std::span<const T> values, int startIndex)
    {
        UpdateBuffer(values.data(), GetTotalSizeOf(values), startIndex * sizeof(T));
    }

    void Bind(int bindingId) const;

    static inline size_t s_NumBytesAllocated = 0;

private:
    uint32_t m_RendererId;
    int m_MaxSize;
};
