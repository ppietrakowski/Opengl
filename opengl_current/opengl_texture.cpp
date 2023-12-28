#include "opengl_texture.h"
#include "error_macros.h"

#include <GL/glew.h>

inline static GLenum ConvertTextureFormatToGL(const TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::kRgb:
        return GL_RGB;
    case TextureFormat::kRgba:
        return GL_RGBA;
    default:
        break;
    }

    return 0x7fffff;
}

OpenGlTexture2D::OpenGlTexture2D(const void* data, const TextureSpecification& specification) :
    m_Width{specification.Width},
    m_Height{specification.Height},
    m_GlFormat(specification.Format == TextureFormat::kRgb ? GL_RGB : GL_RGBA)
{
    GenerateTexture2D(data);
}

OpenGlTexture2D::OpenGlTexture2D(const ImageRgba& image) :
    OpenGlTexture2D{image.GetRawImageData(), TextureSpecification{image.GetWidth(), image.GetHeight(), TextureFormat::kRgba}}
{
}

OpenGlTexture2D::OpenGlTexture2D(const TextureSpecification& specification) :
    m_Width{specification.Width},
    m_Height{specification.Height},
    m_GlFormat(specification.Format == TextureFormat::kRgb ? GL_RGB : GL_RGBA)
{
    GenerateTexture2D(nullptr);
}

OpenGlTexture2D::~OpenGlTexture2D()
{
    glDeleteTextures(1, &m_RendererId);
}

int32_t OpenGlTexture2D::GetWidth() const
{
    return m_Width;
}

int32_t OpenGlTexture2D::GetHeight() const
{
    return m_Height;
}

void OpenGlTexture2D::Bind(int32_t textureUnit) const
{
    glBindTextureUnit(textureUnit, m_RendererId);
}

void OpenGlTexture2D::Unbind(int32_t textureUnit)
{
    glBindTextureUnit(textureUnit, 0);
}

bool OpenGlTexture2D::GotMinimaps() const
{
    return m_bGotMipmaps;
}

void OpenGlTexture2D::GenerateMipmaps()
{
    glGenerateMipmap(m_RendererId);
    m_bGotMipmaps = true;
}

TextureFormat OpenGlTexture2D::GetTextureFormat() const
{
    return m_GlFormat == GL_RGB ? TextureFormat::kRgb : TextureFormat::kRgba;
}

void OpenGlTexture2D::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset)
{
    ERR_FAIL_EXPECTED_TRUE(specification.Width < GetWidth() && specification.Height < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(specification.Width < GetWidth() && specification.Height < GetHeight());

    glTextureSubImage2D(m_RendererId, 0, offset.x, offset.y,
        specification.Width, specification.Height, ConvertTextureFormatToGL(specification.Format), GL_UNSIGNED_BYTE, data);
}

uint32_t OpenGlTexture2D::GetGlFormat() const
{
    return m_GlFormat;
}

void OpenGlTexture2D::GenerateTexture2D(const void* data)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
    glBindTextureUnit(0, m_RendererId); // Binding to texture unit 0 by default

    SetStandardTextureOptions();

    glTextureStorage2D(m_RendererId, 1, GetGlFormat() == GL_RGB ? GL_RGB8 : GL_RGBA8, m_Width, m_Height);

    if (data != nullptr)
    {
        glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, GetGlFormat(), GL_UNSIGNED_BYTE, data);
    }
}

void OpenGlTexture2D::SetStandardTextureOptions()
{
    // Set texture wrapping and filtering options
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}