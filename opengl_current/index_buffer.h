#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

class IIndexBuffer
{
public:
    static std::shared_ptr<IIndexBuffer> Create(const uint32_t* data,
        uint32_t numIndices, bool bDynamic = false);

    static std::shared_ptr<IIndexBuffer> CreateEmpty(uint32_t totalNumIndices);
    virtual ~IIndexBuffer() = default;

public:
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual uint32_t GetNumIndices() const = 0;

    virtual void UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t numElements) = 0;
    virtual void UpdateIndices(const uint32_t* data, uint32_t numElements)
    {
        UpdateIndices(data, 0, numElements);
    }

    virtual bool IsValid() const = 0;
};
