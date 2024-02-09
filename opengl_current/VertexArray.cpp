#include "VertexArray.hpp"
#include "ErrorMacros.hpp"

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

FORCE_INLINE static int GetLookupIndex(VertexAttribute attribute)
{
    return static_cast<int>(attribute.VertexType);
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, AttributesView attributes)
{
    constexpr int MaxAttributes = static_cast<int>(PrimitiveVertexType::MaxPrimitiveVertexType);

    // start index for new buffer
    int attributeStartIndex = STD_ARRAY_NUM_ELEMENTS(m_VertexBuffers);
    int stride = 0;

    constexpr uintptr_t AttributeSizes[MaxAttributes] = {sizeof(int), sizeof(uint32_t), sizeof(float)};
    constexpr GLenum AttributeConversionTable[MaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertexBuffer->Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        int size_index = GetLookupIndex(attribute);
        ASSERT(size_index < MaxAttributes);
        stride += attribute.NumComponents * static_cast<int>(AttributeSizes[size_index]);
        attributes_.emplace_back(attribute);
    }

    uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum is_data_normalized = GL_FALSE;

        int gl_type_index = GetLookupIndex(attribute);
        ASSERT(gl_type_index < MaxAttributes);

        if (attribute.VertexType != PrimitiveVertexType::Float)
        {
            glVertexAttribIPointer(attributeStartIndex, attribute.NumComponents, AttributeConversionTable[gl_type_index],
                stride, reinterpret_cast<const void*>(offset));
        }
        else
        {
            glVertexAttribPointer(attributeStartIndex, attribute.NumComponents, AttributeConversionTable[gl_type_index],
                is_data_normalized, stride, reinterpret_cast<const void*>(offset));
        }

        int size_index = GetLookupIndex(attribute);
        ASSERT(size_index < MaxAttributes);

        offset += attribute.NumComponents * AttributeSizes[size_index];
        attributeStartIndex++;
    }

    m_VertexBuffers.emplace_back(vertexBuffer);
}

uint32_t VertexArray::GetOpenGlIdentifier() const
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

int VertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(m_IndexBuffer, 0);
    return m_IndexBuffer->GetNumIndices();
}

std::shared_ptr<VertexBuffer> VertexArray::GetVertexBufferAt(int index)
{
    return m_VertexBuffers.at(index);
}

std::shared_ptr<IndexBuffer> VertexArray::GetIndexBuffer()
{
    return m_IndexBuffer;
}
