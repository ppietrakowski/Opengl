#pragma once

#include <cstdint>

class UniformBuffer {
public:
    UniformBuffer(int max_size);
    ~UniformBuffer();

    void UpdateBuffer(const void* data, int size_bytes);
    void UpdateBuffer(const void* data, int size_bytes, int offset);

    void Bind(int binding_id) const;

    static inline size_t num_bytes_allocated = 0;

private:
    uint32_t renderer_id_;
    int max_size_;
};
