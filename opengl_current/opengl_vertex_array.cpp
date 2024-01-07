#include "opengl_vertex_array.h"

#include "vertex_array.h"
#include "error_macros.h"

OpenGlVertexArray::OpenGlVertexArray() :
    renderer_id_{0} {
    glBindVertexArray(0);
    glGenVertexArrays(1, &renderer_id_);
}

OpenGlVertexArray::~OpenGlVertexArray() {
    glDeleteVertexArrays(1, &renderer_id_);
}

void OpenGlVertexArray::Bind() const {
    glBindVertexArray(renderer_id_);
}

void OpenGlVertexArray::Unbind() const {
    glBindVertexArray(0);
}

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<std::int32_t>(attribute.vertex_type)
#define GET_GL_TYPE_INDEX(attribute) static_cast<std::int32_t>(attribute.vertex_type)

void OpenGlVertexArray::AddBufferInternal(const std::shared_ptr<VertexBuffer>& vertex_buffer, std::span<const VertexAttribute> attributes) {
    constexpr std::int32_t kMaxAttributes = static_cast<std::int32_t>(PrimitiveVertexType::kMaxPrimitiveVertexType);

    // start index for new buffer
    std::int32_t attribute_start_index = static_cast<std::int32_t>(vertex_buffers_.size());
    std::int32_t stride = 0;

    const uintptr_t kAttributeSizes[kMaxAttributes] = {sizeof(std::int32_t), sizeof(std::uint32_t), sizeof(float)};
    const GLenum kAttributeConversionTable[kMaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertex_buffer->Bind();

    for (const VertexAttribute& attribute : attributes) {
        std::int32_t size_index = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(size_index < kMaxAttributes);
        stride += attribute.num_components * static_cast<std::int32_t>(kAttributeSizes[size_index]);
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes) {
        glEnableVertexAttribArray(attribute_start_index);
        GLenum bDataNormalized = GL_FALSE;

        std::int32_t gl_type_index = GET_GL_TYPE_INDEX(attribute);
        ASSERT(gl_type_index < kMaxAttributes);

        if (attribute.vertex_type != PrimitiveVertexType::kFloat) {
            glVertexAttribIPointer(attribute_start_index, attribute.num_components, kAttributeConversionTable[gl_type_index], stride, reinterpret_cast<const void*>(offset));
        } else {
            glVertexAttribPointer(attribute_start_index, attribute.num_components, kAttributeConversionTable[gl_type_index],
                bDataNormalized, stride, reinterpret_cast<const void*>(offset));
        }

        std::int32_t size_index = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(size_index < kMaxAttributes);

        offset += attribute.num_components * kAttributeSizes[size_index];
        attribute_start_index++;
    }

    vertex_buffers_.emplace_back(vertex_buffer);
}

void OpenGlVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& index_buffer) {
    Bind();

    if (index_buffer_ != nullptr && index_buffer_->IsValid()) {
        index_buffer_->Unbind();
    }

    index_buffer->Bind();
    index_buffer_ = index_buffer;
}

std::int32_t OpenGlVertexArray::GetNumIndices() const {
    ERR_FAIL_EXPECTED_TRUE_V(index_buffer_->IsValid(), 0);
    return index_buffer_->GetNumIndices();
}

std::shared_ptr<VertexBuffer> OpenGlVertexArray::GetVertexBufferAt(std::int32_t index) {
    return vertex_buffers_.at(index);
}

std::shared_ptr<IndexBuffer> OpenGlVertexArray::GetIndexBuffer() {
    return index_buffer_;
}
