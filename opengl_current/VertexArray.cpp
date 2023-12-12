#include "VertexArray.h"

#include <GL/glew.h>
#include "ErrorMacros.h"

VertexArray::VertexArray() :
    _rendererID{ 0 }
{
    glBindVertexArray(0);
    glGenVertexArrays(1, &_rendererID);
}

VertexArray& VertexArray::operator=(VertexArray&& array) noexcept
{
    _rendererID = array._rendererID;
    _indexBuffer = std::move(array._indexBuffer);
    _vertexBuffers = std::move(array._vertexBuffers);

    array._rendererID = 0;
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

#define GET_ATTRIBUTE_INDEX(attribute) static_cast<unsigned int>(attribute.Type)
#define GET_GL_TYPE_INDEX(attribute) static_cast<unsigned int>(attribute.Type)

void VertexArray::AddBuffer(VertexBuffer&& vb, std::span<const VertexAttribute> attributes)
{
    constexpr int MaxAttributes = static_cast<int>(EPrimitiveVertexType::MaxPrimitiveVertexType);

    // start index for new buffer
    int attributeStartIndex = static_cast<int>(_vertexBuffers.size());
    int stride = 0;
    int attributeSizes[MaxAttributes] = { 4, 4, 4 };
    int attributeConversionTable[MaxAttributes] = { GL_INT, GL_UNSIGNED_INT, GL_FLOAT };

    Bind();
    vb.Bind();

    for (const VertexAttribute& attribute : attributes)
    {
        unsigned int sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < MaxAttributes);
        stride += attribute.NumComponents * attributeSizes[sizeIndex];
    }

    std::uintptr_t offset = 0;

    for (const VertexAttribute& attribute : attributes)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum dataNormalized = GL_FALSE;

        int glTypeIndex = GET_GL_TYPE_INDEX(attribute);
        ASSERT(glTypeIndex < MaxAttributes);
        glVertexAttribPointer(attributeStartIndex, attribute.NumComponents, attributeConversionTable[glTypeIndex],
            dataNormalized, stride, reinterpret_cast<const void*>(offset));

        unsigned int sizeIndex = GET_ATTRIBUTE_INDEX(attribute);
        ASSERT(sizeIndex < MaxAttributes);

        offset += attribute.NumComponents * attributeSizes[sizeIndex];
        attributeStartIndex++;
    }

    vb.Unbind();
    _vertexBuffers.push_back(std::move(vb));
}

void VertexArray::SetIndexBuffer(IndexBuffer&& ib)
{
    Bind();

    if (_indexBuffer.IsValid())
    {
        _indexBuffer.Unbind();
    }

    ib.Bind();
    _indexBuffer = std::move(ib);
}

unsigned int VertexArray::GetNumIndices() const
{
    ERR_FAIL_EXPECTED_TRUE_V(_indexBuffer.IsValid(), 0u);
    return _indexBuffer.GetNumIndices();
}
