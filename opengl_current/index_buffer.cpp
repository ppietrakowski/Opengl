#include "index_buffer.h"
#include "render_command.h"

#include <GL/glew.h>

#include "error_macros.h"
#include "renderer_api.h"


IndexBuffer::IndexBuffer(const uint32_t* data, int num_indices, bool dynamic) :
    num_indices_{num_indices}
{
    GLenum buffer_usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindVertexArray(0);

    glGenBuffers(1, &renderer_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(uint32_t), data, buffer_usage);

    RenderCommand::NotifyIndexBufferCreated(num_indices * sizeof(uint32_t));
}

IndexBuffer::IndexBuffer(int max_num_indices) :
    IndexBuffer{nullptr, max_num_indices, true}
{
}

IndexBuffer::~IndexBuffer()
{
    RenderCommand::NotifyIndexBufferDestroyed(num_indices_ * sizeof(uint32_t));
    glDeleteBuffers(1, &renderer_id_);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::UpdateIndices(const uint32_t* data, int offset, int size)
{
    ERR_FAIL_EXPECTED_TRUE_MSG(size <= num_indices_, "Size over declared is causing memory allocation -> may occur memory leak");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_);
    int size_bytes = size * sizeof(uint32_t);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size_bytes, data);
}
