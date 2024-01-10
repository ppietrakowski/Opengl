#include "opengl_vertex_array.h"

#include "vertex_array.h"
#include "error_macros.h"

OpenGlVertexArray::OpenGlVertexArray() :
    m_RendererId{0}
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &m_RendererId);
}

OpenGlVertexArray::~OpenGlVertexArray()
{
    glDeleteVertexArrays(1, &m_RendererId);
}

void OpenGlVertexArray::Bind() const
{
    glBindVertexArray(m_RendererId);
}

void OpenGlVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<std::int32_t>(attribute.VertexType)
#define GET_GL_TYPE_INDEX(attribute) static_cast<std::int32_t>(attribute.VertexType)

void OpenGlVertexArray::AddBufferInternal(const std::shared_ptr<VertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes)
{
    constexpr std::int32_t MaxAttributes = static_cast<std::int32_t>(PrimitiveVertexType::MaxPrimitiveVertexType);

    // start index for new buffer
    std::int32_t attributeStartIndex = static_cast<std::int32_t>(m_VertexBuffers.size());
    std::int32_t stride = 0;

    const uintptr_t AttributeSizes[MaxAttributes] = {sizeof(std::int32_t), sizeof(std::uint32_t), sizeof(float)};
    const GLenum AttributeConversionTable[MaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertexBuffer->Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        std::int32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < MaxAttributes);
        stride += attribute.NumComponents * static_cast<std::int32_t>(AttributeSizes[sizeIndex]);
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum bDataNormalized = GL_FALSE;

        std::int32_t glTypeIndex = GET_GL_TYPE_INDEX(attribute);
        ASSERT(glTypeIndex < MaxAttributes);

        if (attribute.VertexType != PrimitiveVertexType::Float)
        {
            glVertexAttribIPointer(attributeStartIndex, attribute.NumComponents, AttributeConversionTable[glTypeIndex], stride, reinterpret_cast<const void*>(offset));
        } else
        {
            glVertexAttribPointer(attributeStartIndex, attribute.NumComponents, AttributeConversionTable[glTypeIndex],
                bDataNormalized, stride, reinterpret_cast<const void*>(offset));
        }

        std::int32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < MaxAttributes);

        offset += attribute.NumComponents * AttributeSizes[sizeIndex];
        attributeStartIndex++;
    }

    m_VertexBuffers.emplace_back(vertexBuffer);
}

void OpenGlVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
    Bind();

    if (m_IndexBuffer != nullptr)
    {
        m_IndexBuffer->Unbind();
    }

    indexBuffer->Bind();
    m_IndexBuffer = indexBuffer;
}

std::int32_t OpenGlVertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(m_IndexBuffer, 0);
    return m_IndexBuffer->GetNumIndices();
}

std::shared_ptr<VertexBuffer> OpenGlVertexArray::GetVertexBufferAt(std::int32_t index)
{
    return m_VertexBuffers.at(index);
}

std::shared_ptr<IndexBuffer> OpenGlVertexArray::GetIndexBuffer()
{
    return m_IndexBuffer;
}
