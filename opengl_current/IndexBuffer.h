#pragma once

#include "VertexBuffer.h"
#include <cstdint>
#include <span>
#include <vector>

class IndexBuffer
{
public:
    IndexBuffer();
    IndexBuffer(const unsigned int* data, unsigned int numIndices, bool dynamic = false);
    IndexBuffer(unsigned int totalNumIndices);
    IndexBuffer(IndexBuffer&& buffer) noexcept
    {
        *this = std::move(buffer);
    }

    IndexBuffer& operator=(IndexBuffer&& buffer) noexcept;

    ~IndexBuffer();

public:
    void Bind() const;
    void Unbind() const;
    int GetNumIndices() const;
    void UpdateIndices(const unsigned int* data, unsigned int  offset, unsigned int  numElements);
    void UpdateIndices(const unsigned int* data, unsigned int  numElements) { UpdateIndices(data, 0, numElements); }

    bool IsValid() const { return _rendererID != 0; }
    GLuint GetRendererID() const { return _rendererID; }

    void Release();

private:
    GLuint _rendererID;
    unsigned int  _numIndices;
};
