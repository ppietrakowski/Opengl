#include "opengl_vertex_array.h"

#include "vertex_array.h"
#include "error_macros.h"

OpenGlVertexArray::OpenGlVertexArray() :
    RendererId{0}
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &RendererId);
}

OpenGlVertexArray::~OpenGlVertexArray()
{
    glDeleteVertexArrays(1, &RendererId);
}

void OpenGlVertexArray::Bind() const
{
    glBindVertexArray(RendererId);
}

void OpenGlVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<int32_t>(attribute.VertexType)
#define GET_GL_TYPE_INDEX(attribute) static_cast<int32_t>(attribute.VertexType)

void OpenGlVertexArray::AddBufferInternal(const std::shared_ptr<IVertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes)
{
    constexpr int32_t kMaxAttributes = static_cast<int32_t>(PrimitiveVertexType::kMaxPrimitiveVertexType);

    // start index for new buffer
    int32_t attributeStartIndex = static_cast<int32_t>(VertexBuffers.size());
    int32_t stride = 0;

    const uintptr_t kAttributeSizes[kMaxAttributes] = {sizeof(int32_t), sizeof(uint32_t), sizeof(float)};
    const GLenum kAttributeConversionTable[kMaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertexBuffer->Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        int32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < kMaxAttributes);
        stride += attribute.NumComponents * static_cast<int32_t>(kAttributeSizes[sizeIndex]);
    }

    uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum bDataNormalized = GL_FALSE;

        int32_t glTypeIndex = GET_GL_TYPE_INDEX(attribute);
        ASSERT(glTypeIndex < kMaxAttributes);

        if (attribute.VertexType != PrimitiveVertexType::kFloat)
        {
            glVertexAttribIPointer(attributeStartIndex, attribute.NumComponents, kAttributeConversionTable[glTypeIndex], stride, reinterpret_cast<const void*>(offset));
        }
        else
        {
            glVertexAttribPointer(attributeStartIndex, attribute.NumComponents, kAttributeConversionTable[glTypeIndex],
                bDataNormalized, stride, reinterpret_cast<const void*>(offset));
        }

        int32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < kMaxAttributes);

        offset += attribute.NumComponents * kAttributeSizes[sizeIndex];
        attributeStartIndex++;
    }

    VertexBuffers.emplace_back(vertexBuffer);
}

void OpenGlVertexArray::SetIndexBuffer(const std::shared_ptr<IIndexBuffer>& indexBuffer)
{
    Bind();

    if (IndexBuffer != nullptr && IndexBuffer->IsValid())
    {
        IndexBuffer->Unbind();
    }

    indexBuffer->Bind();
    IndexBuffer = indexBuffer;
}

int32_t OpenGlVertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(IndexBuffer->IsValid(), 0);
    return IndexBuffer->GetNumIndices();
}

std::shared_ptr<IVertexBuffer> OpenGlVertexArray::GetVertexBufferAt(int32_t index)
{
    return VertexBuffers.at(index);
}

std::shared_ptr<IIndexBuffer> OpenGlVertexArray::GetIndexBuffer()
{
    return IndexBuffer;
}
