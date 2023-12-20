#include "IndexBuffer.h"
#include "ErrorMacros.h"

#include <GL/glew.h>

IndexBuffer::IndexBuffer() :
    renderer_id_{ 0 },
    num_indices_{ 0 } {}

IndexBuffer::IndexBuffer(const std::uint32_t* data, std::uint32_t num_indices, bool dynamic) :
    num_indices_{ num_indices } {
    GLenum bufferUsage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(std::uint32_t), data, bufferUsage);
}

IndexBuffer::IndexBuffer(std::uint32_t total_num_indices) :
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

void IndexBuffer::UpdateIndices(const std::uint32_t* data, std::uint32_t offset, std::uint32_t num_indices) {
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    std::uint32_t sizeBytes = num_indices * sizeof(std::uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeBytes, data);
}

void IndexBuffer::Release() {
    glDeleteBuffers(1, &renderer_id_);
    renderer_id_ = 0;
    num_indices_ = 0;
}
