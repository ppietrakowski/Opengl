#include "opengl_vertex_buffer.h"
#include "error_macros.h"

#include "vertex_buffer.h"

#include <GL/glew.h>

OpenGlVertexBuffer::OpenGlVertexBuffer(const void* data, std::int32_t size_bytes, bool dynamic) :
    buffer_size_{size_bytes} {
    GLenum buffer_usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ARRAY_BUFFER, size_bytes, data, buffer_usage);
}

OpenGlVertexBuffer::OpenGlVertexBuffer(std::int32_t max_size_bytes) :
    OpenGlVertexBuffer{nullptr, max_size_bytes, true} {
}

OpenGlVertexBuffer::~OpenGlVertexBuffer() {
    glDeleteBuffers(1, &renderer_id_);
    renderer_id_ = 0;
    buffer_size_ = 0;
}

std::int32_t OpenGlVertexBuffer::GetVerticesSizeBytes() const {
    return buffer_size_;
}

bool OpenGlVertexBuffer::IsValid() const {
    return renderer_id_ != 0;
}

void OpenGlVertexBuffer::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
}

void OpenGlVertexBuffer::Unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGlVertexBuffer::UpdateVertices(const void* buffer, std::int32_t offset, std::int32_t size) {
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLintptr>(size), buffer);
}