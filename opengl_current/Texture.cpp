#include "Texture.h"

#include "ErrorMacros.h"
#include "Logging.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION // Ensure that stb_image implementation is included in one source file
#define STBI_FAILURE_USERMSG

extern "C" {
#include "stb_image.h"
}

Texture::~Texture()
{
    glDeleteTextures(1, &_rendererID);
}

void Texture::Bind(std::uint32_t textureUnit) const
{
    glBindTextureUnit(textureUnit, _rendererID);
}

void Texture::Unbind(std::uint32_t textureUnit)
{
    glBindTextureUnit(textureUnit, 0);
}

void Texture::GenerateMipmaps()
{
    glGenerateMipmap(_rendererID);
    _gotMinimaps = true;
}

TextureFormat Texture::GetTextureFormat() const
{
    return _format == GL_RGB ? TextureFormat::Rgb : TextureFormat::Rgba;
}

Texture::Texture(std::uint32_t width, std::uint32_t height, GLenum format) :
    _width{ width },
    _height{ height },
    _format{ format },
    _gotMinimaps{ false },
    _rendererID{ 0 }
{
}

GLenum Texture::GetGlFormat() const
{
    return _format;
}

void Texture::GenerateTexture2D(const void* data)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &_rendererID);
    glBindTextureUnit(0, _rendererID); // Binding to texture unit 0 by default

    SetStandardTextureOptions();

    glTextureStorage2D(_rendererID, 0, GetGlFormat() == GL_RGB ? GL_RGB8 : GL_RGBA8, _width, _height);

    if (data != nullptr)
    {
        glTextureSubImage2D(_rendererID, 0, 0, 0, _width, _height, GetGlFormat(), GL_UNSIGNED_BYTE, data);
    }
}

void Texture::SetStandardTextureOptions()
{
    // Set texture wrapping and filtering options
    glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::Texture2D(const std::string& filepath) :
    Texture{ 0, 0, GL_RGB }
{
    LoadFromFile(filepath);
}

Texture2D::Texture2D(const void* data, std::uint32_t width, std::uint32_t height, TextureFormat format) :
    Texture{ width, height, format == TextureFormat::Rgb ? static_cast<GLenum>(GL_RGB) : static_cast<GLenum>(GL_RGBA) }
{
    GenerateTexture2D(data);
}

Texture2D::Texture2D(std::uint32_t width, std::uint32_t height, TextureFormat format) :
    Texture2D{ nullptr, width, height, format }
{
}

void Texture2D::SetData(const void* data, glm::uvec2 size, glm::uvec2 offset)
{
    ERR_FAIL_EXPECTED_TRUE(size.x < GetWidth() && size.y < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(offset.x < GetWidth() && offset.y < GetHeight());

    glTextureSubImage2D(_rendererID, 0, offset.x, offset.y, size.x, size.y, GetGlFormat(), GL_UNSIGNED_BYTE, data);
}

void Texture2D::LoadFromFile(const std::string& filepath)
{
    // Load texture using stb_image
    stbi_set_flip_vertically_on_load(GL_TRUE); // Flip the image vertically if needed
    std::int32_t width;
    std::int32_t height;
    std::int32_t channels;
    std::uint8_t* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    if (data != nullptr)
    {
        _width = static_cast<std::uint32_t>(width);
        _height = static_cast<std::uint32_t>(height);
        _format = channels == 3 ? GL_RGB : GL_RGBA;
        GenerateTexture2D(data);
    }
    else
    {
        throw std::runtime_error{ "Failed to load texture " + filepath };
    }
    stbi_image_free(data);

    glBindTextureUnit(0, 0); // Unbind texture from texture unit 0
}
