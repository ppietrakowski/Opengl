#include "VertexArray.h"

#include <GL/glew.h>
#include "ErrorMacros.h"

VertexArray::VertexArray() :
    _rendererID{ 0 }
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &_rendererID);
}

VertexArray& VertexArray::operator=(VertexArray&& tempVertexArray) noexcept
{
    _rendererID = tempVertexArray._rendererID;
    _indexBuffer = std::move(tempVertexArray._indexBuffer);
    _vertexBuffers = std::move(tempVertexArray._vertexBuffers);

    tempVertexArray._rendererID = 0;
    return *this;
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &_rendererID);
}

void VertexArray::Bind() const
{
    glBindVertexArray(_rendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<std::uint32_t>(attribute.Type)
#define GET_GL_TYPE_INDEX(attribute) static_cast<std::uint32_t>(attribute.Type)

void VertexArray::AddBuffer(VertexBuffer&& vertexBuffer, std::span<const VertexAttribute> attributes)
{
    constexpr std::uint32_t MaxAttributes = static_cast<std::uint32_t>(PrimitiveVertexType::MaxPrimitiveVertexType);

    // start index for new buffer
    std::uint32_t attributeStartIndex = static_cast<std::uint32_t>(_vertexBuffers.size());
    std::uint32_t stride = 0;


    std::uint32_t attributeSizes[MaxAttributes] = { sizeof(std::int32_t), sizeof(std::uint32_t), sizeof(float) };
    std::uint32_t attributeConversionTable[MaxAttributes] = { GL_INT, GL_UNSIGNED_INT, GL_FLOAT };

    Bind();
    vertexBuffer.Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        std::uint32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < MaxAttributes);
        stride += attribute.NumComponents * attributeSizes[sizeIndex];
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum dataNormalized = GL_FALSE;

        std::uint32_t glTypeIndex = GET_GL_TYPE_INDEX(attribute);
        ASSERT(glTypeIndex < MaxAttributes);
        glVertexAttribPointer(attributeStartIndex, attribute.NumComponents, attributeConversionTable[glTypeIndex],
            dataNormalized, stride, reinterpret_cast<const void*>(offset));

        std::uint32_t sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < MaxAttributes);

        offset += attribute.NumComponents * attributeSizes[sizeIndex];
        attributeStartIndex++;
    }

    vertexBuffer.Unbind();
    _vertexBuffers.push_back(std::move(vertexBuffer));
}

void VertexArray::SetIndexBuffer(IndexBuffer&& indexBuffer)
{
    Bind();

    if (_indexBuffer.IsValid())
    {
        _indexBuffer.Unbind();
    }

    indexBuffer.Bind();
    _indexBuffer = std::move(indexBuffer);
}

std::uint32_t VertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(_indexBuffer.IsValid(), 0u);
    return _indexBuffer.GetNumIndices();
}
