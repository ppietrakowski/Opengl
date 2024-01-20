#include "uniform_buffer.h"

#include <GL/glew.h>

UniformBuffer::UniformBuffer(int max_size) :
    max_size_(max_size) {
    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_UNIFORM_BUFFER, renderer_id_);

    glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)max_size, nullptr, GL_DYNAMIC_DRAW);
    num_bytes_allocated += max_size;
}

UniformBuffer::~UniformBuffer() {
    glDeleteBuffers(1, &renderer_id_);
    num_bytes_allocated -= max_size_;
}

void UniformBuffer::UpdateBuffer(const void* data, int size_bytes) {
    glBindBuffer(GL_UNIFORM_BUFFER, renderer_id_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size_bytes, data);
}

void UniformBuffer::UpdateBuffer(const void* data, int size_bytes, int offset) {
    glBindBuffer(GL_UNIFORM_BUFFER, renderer_id_);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size_bytes, data);
}

void UniformBuffer::Bind(int binding_id) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_id, renderer_id_);
}
