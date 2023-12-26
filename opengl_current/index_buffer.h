#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

class IndexBuffer {
public:
    static std::shared_ptr<IndexBuffer> Create(const uint32_t* data, 
        uint32_t num_indices, bool dynamic = false);

    static std::shared_ptr<IndexBuffer> CreateEmpty(uint32_t total_num_indices);
    virtual ~IndexBuffer() = default;

public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual uint32_t GetNumIndices() const = 0;

    virtual void UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t num_elements) = 0;
    virtual void UpdateIndices(const uint32_t* data, uint32_t num_elements) {
        UpdateIndices(data, 0, num_elements);
    }

    virtual bool IsValid() const = 0;
};
