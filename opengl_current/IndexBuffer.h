#pragma once

#include "VertexBuffer.h"
#include <cstdint>
#include <span>
#include <vector>

class IndexBuffer
{
public:
    IndexBuffer();
    IndexBuffer(const std::uint32_t* data, std::uint32_t numIndices, bool dynamic = false);
    IndexBuffer(std::uint32_t totalNumIndices);
    IndexBuffer(IndexBuffer&& buffer) noexcept
    {
        *this = std::move(buffer);
    }

    IndexBuffer& operator=(IndexBuffer&& buffer) noexcept;

    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    std::uint32_t GetNumIndices() const { return _numIndices; }

    void UpdateIndices(const std::uint32_t* data, std::uint32_t offset, std::uint32_t numElements);
    void UpdateIndices(const std::uint32_t* data, std::uint32_t numElements) { UpdateIndices(data, 0, numElements); }

    bool IsValid() const { return _rendererID != 0; }
    GLuint GetRendererID() const { return _rendererID; }

    void Release();

private:
    GLuint _rendererID;
    std::uint32_t _numIndices;
};
