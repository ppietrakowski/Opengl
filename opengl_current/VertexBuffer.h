#pragma once

#include <gl/glew.h>

#include <cstdint>
#include <span>
#include <vector>
#include <memory>

#include "Core.h"

class VertexBuffer
{
public:
    VertexBuffer();
    VertexBuffer(const void* data, std::uint32_t sizeBytes, bool dynamic = false);
    VertexBuffer(std::uint32_t maxSizeBytes);
    VertexBuffer(VertexBuffer&& tempVertexBuffer) noexcept
    {
        *this = std::move(tempVertexBuffer);
    }

    VertexBuffer& operator=(VertexBuffer&& tempVertexBuffer) noexcept;
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;
    void UpdateVertices(const void* data, std::uint32_t offset, std::uint32_t size);
    void UpdateVertices(const void* data, std::uint32_t size) { UpdateVertices(data, 0, size); }

    std::uint32_t GetVerticesSizeBytes() const { return _bufferSize; }

    bool IsValid() const { return _rendererID != 0; }
    GLuint GetRendererID() const { return _rendererID; }

    void Release();

private:
    GLuint _rendererID;
    std::uint32_t _bufferSize;
};