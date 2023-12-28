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

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<uint32_t>(attribute.VertexType)
#define GET_GL_TYPE_INDEX(attribute) static_cast<uint32_t>(attribute.VertexType)

void OpenGlVertexArray::AddBufferInternal(const std::shared_ptr<IVertexBuffer>& vertexBuffer, std::span<const VertexAttribute> attributes)
{
    constexpr uint32_t kMaxAttributes = static_cast<uint32_t>(PrimitiveVertexType::kMaxPrimitiveVertexType);

    // start index for new buffer
    uint32_t attributeStartIndex = static_cast<uint32_t>(m_VertexBuffers.size());
    uint32_t stride = 0;

    const uintptr_t kAttributeSizes[kMaxAttributes] = {sizeof(int32_t), sizeof(uint32_t), sizeof(float)};
    const uint32_t kAttributeConversionTable[kMaxAttributes] = {GL_INT, GL_UNSIGNED_INT, GL_FLOAT};

    Bind();
    vertexBuffer->Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        uint32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < kMaxAttributes);
        stride += attribute.NumComponents * static_cast<uint32_t>(kAttributeSizes[sizeIndex]);
    }

    uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum bDataNormalized = GL_FALSE;

        uint32_t glTypeIndex = GET_GL_TYPE_INDEX(attribute);
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

        uint32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < kMaxAttributes);

        offset += attribute.NumComponents * kAttributeSizes[sizeIndex];
        attributeStartIndex++;
    }

    m_VertexBuffers.emplace_back(vertexBuffer);
}

void OpenGlVertexArray::SetIndexBuffer(const std::shared_ptr<IIndexBuffer>& indexBuffer)
{
    Bind();

    if (m_IndexBuffer != nullptr && m_IndexBuffer->IsValid())
    {
        m_IndexBuffer->Unbind();
    }

    indexBuffer->Bind();
    m_IndexBuffer = indexBuffer;
}

uint32_t OpenGlVertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(m_IndexBuffer->IsValid(), 0u);
    return m_IndexBuffer->GetNumIndices();
}

std::shared_ptr<IVertexBuffer> OpenGlVertexArray::GetVertexBufferAt(uint32_t index)
{
    return m_VertexBuffers.at(index);
}

std::shared_ptr<IIndexBuffer> OpenGlVertexArray::GetIndexBuffer()
{
    return m_IndexBuffer;
}
