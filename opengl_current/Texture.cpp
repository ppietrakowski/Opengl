#include "Texture.h"

#include "ErrorMacros.h"
#include "Logging.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION // Ensure that stb_image implementation is included in one source file
#define STBI_FAILURE_USERMSG

extern "C" {
#include "stb_image.h"
}

Texture::~Texture() {
    glDeleteTextures(1, &renderer_id_);
}

void Texture::Bind(uint32_t texture_unit) const {
    glBindTextureUnit(texture_unit, renderer_id_);
}

void Texture::Unbind(uint32_t texture_unit) {
    glBindTextureUnit(texture_unit, 0);
}

void Texture::GenerateMipmaps() {
    glGenerateMipmap(renderer_id_);
    got_mipmaps_ = true;
}

TextureFormat Texture::GetTextureFormat() const {
    return format_ == GL_RGB ? TextureFormat::kRgb : TextureFormat::kRgba;
}

Texture::Texture(uint32_t width, uint32_t height, GLenum format) :
    width_{ width },
    height_{ height },
    format_{ format },
    got_mipmaps_{ false },
    renderer_id_{ 0 } {}

GLenum Texture::GetGlFormat() const {
    return format_;
}

void Texture::GenerateTexture2D(const void* data) {
    glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);
    glBindTextureUnit(0, renderer_id_); // Binding to texture unit 0 by default

    SetStandardTextureOptions();

    glTextureStorage2D(renderer_id_, 1, GetGlFormat() == GL_RGB ? GL_RGB8 : GL_RGBA8, width_, height_);

    if (data != nullptr) {
        glTextureSubImage2D(renderer_id_, 0, 0, 0, width_, height_, GetGlFormat(), GL_UNSIGNED_BYTE, data);
    }
}

void Texture::SetStandardTextureOptions() {
    // Set texture wrapping and filtering options
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::Texture2D(const std::string& file_path) :
    Texture{ 0, 0, GL_RGB } {
    LoadFromFile(file_path);
}

Texture2D::Texture2D(const void* data, uint32_t width, uint32_t height, TextureFormat format) :
    Texture{ width, height, format == TextureFormat::kRgb ? static_cast<GLenum>(GL_RGB) : static_cast<GLenum>(GL_RGBA) } {
    GenerateTexture2D(data);
}

Texture2D::Texture2D(uint32_t width, uint32_t height, TextureFormat format) :
    Texture2D{ nullptr, width, height, format } {}

void Texture2D::SetData(const void* data, glm::uvec2 size, glm::uvec2 offset) {
    ERR_FAIL_EXPECTED_TRUE(size.x < GetWidth() && size.y < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(offset.x < GetWidth() && offset.y < GetHeight());

    glTextureSubImage2D(renderer_id_, 0, offset.x, offset.y, size.x, size.y, GetGlFormat(), GL_UNSIGNED_BYTE, data);
}

void Texture2D::LoadFromFile(const std::string& file_path) {
    // Load texture using stb_image
    stbi_set_flip_vertically_on_load(GL_TRUE); // Flip the image vertically if needed
    int32_t width;
    int32_t height;
    int32_t channels;
    uint8_t* data = stbi_load(file_path.c_str(), &width, &height, &channels, 0);

    if (data != nullptr) {
        width_ = static_cast<uint32_t>(width);
        height_ = static_cast<uint32_t>(height);
        format_ = channels == 3 ? GL_RGB : GL_RGBA;
        GenerateTexture2D(data);
    } else {
        throw std::runtime_error{ "Failed to load texture " + file_path };
    }
    stbi_image_free(data);

    glBindTextureUnit(0, 0); // Unbind texture from texture unit 0
}
