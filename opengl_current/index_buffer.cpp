#include "index_buffer.h"
#include "error_macros.h"

#include <GL/glew.h>

IndexBuffer::IndexBuffer() :
    renderer_id_{ 0 },
    num_indices_{ 0 } {}

IndexBuffer::IndexBuffer(const uint32_t* data, uint32_t num_indices, bool dynamic) :
    num_indices_{ num_indices } {
    GLenum buffer_usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(uint32_t), data, buffer_usage);
}

IndexBuffer::IndexBuffer(uint32_t total_num_indices) :
    IndexBuffer{ nullptr, total_num_indices, true } {}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& buffer) noexcept {
    num_indices_ = buffer.num_indices_;
    renderer_id_ = buffer.renderer_id_;
    buffer.num_indices_ = 0;
    buffer.renderer_id_ = 0;
    return *this;
}

IndexBuffer::~IndexBuffer() {
    Release();
}

void IndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
}

void IndexBuffer::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::UpdateIndices(const uint32_t* data, uint32_t offset, uint32_t num_indices) {
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    uint32_t size_bytes = num_indices * sizeof(uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size_bytes, data);
}

void IndexBuffer::Release() {
    glDeleteBuffers(1, &renderer_id_);
    renderer_id_ = 0;
    num_indices_ = 0;
}
