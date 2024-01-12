#include "Texture.h"

#include "error_macros.h"
#include "logging.h"
#include "renderer_api.h"

#include <GL/glew.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION // Ensure that stb_image implementation is included in one source file
#define STBI_FAILURE_USERMSG

extern "C" {
#include "stb_image.h"
}

static void StbiDeleter(std::uint8_t* bytes)
{
    stbi_image_free(bytes);
}

inline static GLenum ConvertTextureFormatToGL(const TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::Rgb:
        return GL_RGB;
    case TextureFormat::Rgba:
        return GL_RGBA;
    default:
        break;
    }

    return 0x7fffff;
}

Texture2D::Texture2D(const std::filesystem::path& filePath)
{
    // Flip the image vertically if needed
    stbi_set_flip_vertically_on_load(1);

    std::string path = filePath.string();
    stbi_image_data_t imageData = stbi_load_from_filepath(path.c_str(), STBI_rgb_alpha);

    if (imageData.data == nullptr)
    {
        throw std::runtime_error{"Failed to load texture " + path};
    }

    m_Width = imageData.width;
    m_Height = imageData.height;
    m_GlFormat = imageData.num_channels == 3 ? GL_RGB : GL_RGBA;

    GenerateTexture2D(imageData.data);

    STBI_FREE(imageData.data);
}

Texture2D::Texture2D(const void* data, const TextureSpecification& specification) :
    m_Width{specification.Width},
    m_Height{specification.Height},
    m_GlFormat(specification.Format == TextureFormat::Rgb ? GL_RGB : GL_RGBA)
{
    GenerateTexture2D(data);
}

Texture2D::Texture2D(const ImageRgba& image) :
    Texture2D{image.GetRawImageData(), TextureSpecification{image.GetWidth(), image.GetHeight(), TextureFormat::Rgba}}
{
}

Texture2D::Texture2D(const TextureSpecification& specification) :
    m_Width{specification.Width},
    m_Height{specification.Height},
    m_GlFormat(specification.Format == TextureFormat::Rgb ? GL_RGB : GL_RGBA)
{
    GenerateTexture2D(nullptr);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_RendererId);
}

std::int32_t Texture2D::GetWidth() const
{
    return m_Width;
}

std::int32_t Texture2D::GetHeight() const
{
    return m_Height;
}

void Texture2D::Bind(std::uint32_t textureUnit) const
{
    glBindTextureUnit(textureUnit, m_RendererId);
}

void Texture2D::Unbind(std::uint32_t textureUnit)
{
    glBindTextureUnit(textureUnit, 0);
}

bool Texture2D::GotMinimaps() const
{
    return m_bHasMipmaps;
}

void Texture2D::GenerateMipmaps()
{
    glGenerateMipmap(m_RendererId);
    m_bHasMipmaps = true;
}

TextureFormat Texture2D::GetTextureFormat() const
{
    return m_GlFormat == GL_RGB ? TextureFormat::Rgb : TextureFormat::Rgba;
}

void Texture2D::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset)
{
    ERR_FAIL_EXPECTED_TRUE(specification.Width < GetWidth() && specification.Height < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(specification.Width < GetWidth() && specification.Height < GetHeight());

    glTextureSubImage2D(m_RendererId, 0, offset.x, offset.y,
        specification.Width, specification.Height, ConvertTextureFormatToGL(specification.Format), GL_UNSIGNED_BYTE, data);
}

uint32_t Texture2D::GetGlFormat() const
{
    return m_GlFormat;
}

void Texture2D::GenerateTexture2D(const void* data)
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

void Texture2D::SetStandardTextureOptions()
{
    // Set texture wrapping and filtering options
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

ImageRgba LoadRgbaImageFromMemory(const void* data, std::int32_t length)
{
    stbi_image_data_t imageData = stbi_load_from_memory_rgba(reinterpret_cast<const std::uint8_t*>(data), length);
    return ImageRgba{imageData.data, imageData.width, imageData.height, &StbiDeleter};
}
