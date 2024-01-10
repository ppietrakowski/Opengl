#pragma once

#include "vertex_array.h"
#include <GL/glew.h>

class OpenGlVertexArray : public VertexArray
{
public:
    OpenGlVertexArray();
    ~OpenGlVertexArray();

public:
    void Bind() const override;
    void Unbind() const override;

    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

    std::int32_t GetNumIndices() const override;

    std::shared_ptr<VertexBuffer> GetVertexBufferAt(std::int32_t index) override;
    std::shared_ptr<IndexBuffer> GetIndexBuffer() override;
    void AddBufferInternal(const std::shared_ptr<VertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes) override;

private:
    GLuint m_RendererId;
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
};

