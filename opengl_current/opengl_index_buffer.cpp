#include "opengl_index_buffer.h"
#include "error_macros.h"

OpenGlIndexBuffer::OpenGlIndexBuffer() :
    renderer_id_{0},
    num_indices_{0} {
}

OpenGlIndexBuffer::OpenGlIndexBuffer(const std::uint32_t* data, std::int32_t num_indices, bool bDynamic) :
    num_indices_{num_indices} {
    GLenum buffer_usage = bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(std::uint32_t), data, buffer_usage);
}

OpenGlIndexBuffer::OpenGlIndexBuffer(std::int32_t total_num_indices) :
    OpenGlIndexBuffer{nullptr, total_num_indices, true} {
}

OpenGlIndexBuffer::~OpenGlIndexBuffer() {
    glDeleteBuffers(1, &renderer_id_);
    renderer_id_ = 0;
    num_indices_ = 0;
}

std::int32_t OpenGlIndexBuffer::GetNumIndices() const {
    return num_indices_;
}

void OpenGlIndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
}

void OpenGlIndexBuffer::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGlIndexBuffer::UpdateIndices(const uint32_t* data, std::int32_t offset, std::int32_t size) {
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    std::int32_t size_bytes = size * sizeof(std::uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size_bytes, data);
}

bool OpenGlIndexBuffer::IsValid() const {
    return renderer_id_ != 0;
}
