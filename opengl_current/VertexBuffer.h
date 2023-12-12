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
    VertexBuffer(const void* data, unsigned int size, bool dynamic = false);
    VertexBuffer(unsigned int size);
    VertexBuffer(VertexBuffer&& buffer) noexcept
    {
        *this = std::move(buffer);
    }

    VertexBuffer& operator=(VertexBuffer&& buffer) noexcept;
    ~VertexBuffer();

public:

    void Bind() const;
    void Unbind() const;
    void UpdateVertices(const void* data, unsigned int offset, unsigned int size);
    void UpdateVertices(const void* data, unsigned int size) { UpdateVertices(data, 0, size); }

    unsigned int GetVerticesSizeBytes() const;

    bool IsValid() const { return _rendererID != 0; }
    GLuint GetRendererID() const { return _rendererID; }

    void Release();

private:
    GLuint _rendererID;
    unsigned int _bufferSize;
};