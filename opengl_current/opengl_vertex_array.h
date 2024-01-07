#pragma once

#include "vertex_array.h"
#include <GL/glew.h>

class OpenGlVertexArray : public VertexArray {
public:
    OpenGlVertexArray();
    ~OpenGlVertexArray();

public:
    void Bind() const override;
    void Unbind() const override;

    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) override;

    std::int32_t GetNumIndices() const override;

    std::shared_ptr<VertexBuffer> GetVertexBufferAt(std::int32_t index) override;
    std::shared_ptr<IndexBuffer> GetIndexBuffer() override;
    void AddBufferInternal(const std::shared_ptr<VertexBuffer>& vertex_buffer, std::span<const VertexAttribute> attributes) override;

private:
    GLuint renderer_id_;
    std::vector<std::shared_ptr<VertexBuffer>> vertex_buffers_;
    std::shared_ptr<IndexBuffer> index_buffer_;
};

