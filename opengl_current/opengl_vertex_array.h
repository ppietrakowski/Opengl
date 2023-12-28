#pragma once

#include "vertex_array.h"
#include <GL/glew.h>

class OpenGlVertexArray : public IVertexArray
{
public:
    OpenGlVertexArray();
    ~OpenGlVertexArray();

public:
    void Bind() const override;
    void Unbind() const override;

    void SetIndexBuffer(const std::shared_ptr<IIndexBuffer>& indexBuffer) override;

    uint32_t GetNumIndices() const override;

    std::shared_ptr<IVertexBuffer> GetVertexBufferAt(uint32_t index) override;
    std::shared_ptr<IIndexBuffer> GetIndexBuffer() override;
    void AddBufferInternal(const std::shared_ptr<IVertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes) override;

private:
    GLuint m_RendererId;
    std::vector<std::shared_ptr<IVertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IIndexBuffer> m_IndexBuffer;
};

