#include "VertexArray.h"

#include <GL/glew.h>
#include "ErrorMacros.h"

VertexArray::VertexArray() :
    renderer_id_{ 0 }
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &renderer_id_);
}

VertexArray& VertexArray::operator=(VertexArray&& temp_vertex_array) noexcept
{
    renderer_id_ = temp_vertex_array.renderer_id_;
    index_buffer_ = std::move(temp_vertex_array.index_buffer_);
    vertex_buffers_ = std::move(temp_vertex_array.vertex_buffers_);

    temp_vertex_array.renderer_id_ = 0;
    return *this;
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &renderer_id_);
}

void VertexArray::Bind() const
{
    glBindVertexArray(renderer_id_);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<std::uint32_t>(attribute.vertex_type)
#define GET_GL_TYPE_INDEX(attribute) static_cast<std::uint32_t>(attribute.vertex_type)

void VertexArray::AddBufferInternal(VertexBuffer&& vertex_buffer, std::span<const VertexAttribute> attributes)
{
    constexpr std::uint32_t kMaxAttributes = static_cast<std::uint32_t>(PrimitiveVertexType::kMaxPrimitiveVertexType);

    // start index for new buffer
    std::uint32_t attribute_start_index = static_cast<std::uint32_t>(vertex_buffers_.size());
    std::uint32_t stride = 0;

    const std::uint32_t kAttributeSizes[kMaxAttributes] = { sizeof(std::int32_t), sizeof(std::uint32_t), sizeof(float) };
    const std::uint32_t kAttributeConversionTable[kMaxAttributes] = { GL_INT, GL_UNSIGNED_INT, GL_FLOAT };

    Bind();
    vertex_buffer.Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        std::uint32_t size_index = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(size_index < kMaxAttributes);
        stride += attribute.num_components * kAttributeSizes[size_index];
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attribute_start_index);
        GLenum data_normalized = GL_FALSE;

        std::uint32_t gl_type_index = GET_GL_TYPE_INDEX(attribute);
        ASSERT(gl_type_index < kMaxAttributes);

        glVertexAttribPointer(attribute_start_index, attribute.num_components, kAttributeConversionTable[gl_type_index],
            data_normalized, stride, reinterpret_cast<const void*>(offset));

        std::uint32_t size_index = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(size_index < kMaxAttributes);

        offset += attribute.num_components * kAttributeSizes[size_index];
        attribute_start_index++;
    }

    vertex_buffer.Unbind();
    vertex_buffers_.emplace_back(std::move(vertex_buffer));
}

void VertexArray::SetIndexBuffer(IndexBuffer&& index_buffer)
{
    Bind();

    if (index_buffer_.IsValid())
    {
        index_buffer_.Unbind();
    }

    index_buffer.Bind();
    index_buffer_ = std::move(index_buffer);
}

std::uint32_t VertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(index_buffer_.IsValid(), 0u);
    return index_buffer_.GetNumIndices();
}
