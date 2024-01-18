#pragma once

#include <cstdint>

class UniformBuffer
{
public:
    UniformBuffer(size_t maxSize);
    ~UniformBuffer();

    void UpdateBuffer(const void* data, size_t size_bytes);
    void UpdateBuffer(const void* data, size_t size_bytes, size_t offset);

    void Bind(std::int32_t binding_id) const;

    static inline size_t num_bytes_allocated = 0;

private:
    std::uint32_t renderer_id_;
    size_t max_size_;
};
