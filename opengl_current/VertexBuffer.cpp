#include "VertexBuffer.h"

#include "ErrorMacros.h"
#include <GL/glew.h>

VertexBuffer::VertexBuffer() :
    buffer_size_{ 0 },
    renderer_id_{ 0 }
{
}

VertexBuffer::VertexBuffer(const void* data, std::uint32_t size_bytes, bool dynamic) :
    buffer_size_{ size_bytes }
{
    GLenum buffer_usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ARRAY_BUFFER, size_bytes, data, buffer_usage);
}

VertexBuffer::VertexBuffer(std::uint32_t max_size_bytes) :
    VertexBuffer{ nullptr, max_size_bytes, true }
{
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& temp_vertex_buffer) noexcept
{
    buffer_size_ = temp_vertex_buffer.buffer_size_;
    renderer_id_ = temp_vertex_buffer.renderer_id_;

    temp_vertex_buffer.renderer_id_ = 0;
    temp_vertex_buffer.buffer_size_ = 0;
    return *this;
}

VertexBuffer::~VertexBuffer()
{
    Release();
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UpdateVertices(const void* data, std::uint32_t offset, std::uint32_t size)
{
    ERR_FAIL_EXPECTED_TRUE(IsValid());

    glBindBuffer(GL_ARRAY_BUFFER, renderer_id_);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLintptr>(size), data);
}

void VertexBuffer::Release()
{
    glDeleteBuffers(1, &renderer_id_);
    renderer_id_ = 0;
    buffer_size_ = 0;
}
