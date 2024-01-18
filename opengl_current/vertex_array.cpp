#include "vertex_array.h"
#include "error_macros.h"

#include <GL/glew.h>

VertexArray::VertexArray() :
    renderer_id_{0}
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &renderer_id_);
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

static FORCE_INLINE std::int32_t GetLookupIndex(VertexAttribute attribute)
{
    return static_cast<std::int32_t>(attribute.vertex_type);
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertex_buffer, AttributesView attributes)
{
    constexpr std::int32_t kMaxAttributes = static_cast<std::int32_t>(PrimitiveVertexType::kMaxPrimitiveVertexType);

    // start index for new buffer
    std::int32_t attribute_start_index = static_cast<std::int32_t>(vertex_buffers_.size());
    std::int32_t stride = 0;

    const std::uintptr_t kAttributeSizes[kMaxAttributes] = {sizeof(std::int32_t), sizeof(std::uint32_t), sizeof(float)};
    const GLenum kAttributeConversionTable[kMaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertex_buffer->Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        std::int32_t size_index = GetLookupIndex(attribute);
        ASSERT(size_index < kMaxAttributes);
        stride += attribute.num_components * static_cast<std::int32_t>(kAttributeSizes[size_index]);
        attributes_.emplace_back(attribute);
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attribute_start_index);
        GLenum is_data_normalized = GL_FALSE;

        std::int32_t gl_type_index = GetLookupIndex(attribute);
        ASSERT(gl_type_index < kMaxAttributes);

        if (attribute.vertex_type != PrimitiveVertexType::kFloat)
        {
            glVertexAttribIPointer(attribute_start_index, attribute.num_components, kAttributeConversionTable[gl_type_index], stride, reinterpret_cast<const void*>(offset));
        } else
        {
            glVertexAttribPointer(attribute_start_index, attribute.num_components, kAttributeConversionTable[gl_type_index],
                is_data_normalized, stride, reinterpret_cast<const void*>(offset));
        }

        std::int32_t size_index = GetLookupIndex(attribute);
        ASSERT(size_index < kMaxAttributes);

        offset += attribute.num_components * kAttributeSizes[size_index];
        attribute_start_index++;
    }

    vertex_buffers_.emplace_back(vertex_buffer);
}

std::uint32_t VertexArray::GetOpenGlIdentifier() const
{
    return renderer_id_;
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer)
{
    Bind();

    if (index_buffer_ != nullptr)
    {
        index_buffer_->Unbind();
    }

    index_buffer->Bind();
    index_buffer_ = index_buffer;
}

std::int32_t VertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(index_buffer_, 0);
    return index_buffer_->GetNumIndices();
}

std::shared_ptr<VertexBuffer> VertexArray::GetVertexBufferAt(std::int32_t index)
{
    return vertex_buffers_.at(index);
}

std::shared_ptr<IndexBuffer> VertexArray::GetIndexBuffer()
{
    return index_buffer_;
}
