#include "vertex_array.h"
#include "error_macros.h"

#include <GL/glew.h>

VertexArray::VertexArray() :
    m_RendererId{0}
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &m_RendererId);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_RendererId);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererId);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

static FORCE_INLINE std::int32_t GetLookupIndex(VertexAttribute attribute)
{
    return static_cast<std::int32_t>(attribute.VertexType);
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, AttributesView attributes)
{
    constexpr std::int32_t MaxAttributes = static_cast<std::int32_t>(PrimitiveVertexType::MaxPrimitiveVertexType);

    // start index for new buffer
    std::int32_t attributeStartIndex = static_cast<std::int32_t>(m_VertexBuffers.size());
    std::int32_t stride = 0;

    const std::uintptr_t AttributeSizes[MaxAttributes] = {sizeof(std::int32_t), sizeof(std::uint32_t), sizeof(float)};
    const GLenum AttributeConversionTable[MaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertexBuffer->Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        std::int32_t sizeIndex = GetLookupIndex(attribute);
        ASSERT(sizeIndex < MaxAttributes);
        stride += attribute.NumComponents * static_cast<std::int32_t>(AttributeSizes[sizeIndex]);
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum bDataNormalized = GL_FALSE;

        std::int32_t glTypeIndex = GetLookupIndex(attribute);
        ASSERT(glTypeIndex < MaxAttributes);

        if (attribute.VertexType != PrimitiveVertexType::Float)
        {
            glVertexAttribIPointer(attributeStartIndex, attribute.NumComponents, AttributeConversionTable[glTypeIndex], stride, reinterpret_cast<const void*>(offset));
        } else
        {
            glVertexAttribPointer(attributeStartIndex, attribute.NumComponents, AttributeConversionTable[glTypeIndex],
                bDataNormalized, stride, reinterpret_cast<const void*>(offset));
        }

        std::int32_t sizeIndex = GetLookupIndex(attribute);
        ASSERT(sizeIndex < MaxAttributes);

        offset += attribute.NumComponents * AttributeSizes[sizeIndex];
        attributeStartIndex++;
    }

    m_VertexBuffers.emplace_back(vertexBuffer);
}

std::uint32_t VertexArray::GetOpenGlIdentifier() const
{
    return m_RendererId;
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
    Bind();

    if (m_IndexBuffer != nullptr)
    {
        m_IndexBuffer->Unbind();
    }

    indexBuffer->Bind();
    m_IndexBuffer = indexBuffer;
}

std::int32_t VertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(m_IndexBuffer, 0);
    return m_IndexBuffer->GetNumIndices();
}

std::shared_ptr<VertexBuffer> VertexArray::GetVertexBufferAt(std::int32_t index)
{
    return m_VertexBuffers.at(index);
}

std::shared_ptr<IndexBuffer> VertexArray::GetIndexBuffer()
{
    return m_IndexBuffer;
}
