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

static void StbiDeleter(uint8_t* bytes) {
    stbi_image_free(bytes);
}

inline static GLenum ConvertTextureFormatToGL(const TextureFormat format) {
    switch (format) {
    case TextureFormat::kRgb:
        return GL_RGB;
    case TextureFormat::kRgba:
        return GL_RGBA;
    default:
        break;
    }

    return 0x7fffff;
}

Texture2D::Texture2D(const std::filesystem::path& file_path) {
    // Flip the image vertically if needed
    stbi_set_flip_vertically_on_load(1);

    std::string path = file_path.string();
    stbi_image_data_t image_data = stbi_load_from_filepath(path.c_str(), STBI_rgb_alpha);

    if (image_data.data == nullptr) {
        throw std::runtime_error{"Failed to load texture " + path};
    }

    width_ = image_data.width;
    heigth_ = image_data.height;
    gl_format_ = image_data.num_channels == 3 ? GL_RGB : GL_RGBA;

    GenerateTexture2D(image_data.data);

    STBI_FREE(image_data.data);
}

Texture2D::Texture2D(const void* data, const TextureSpecification& specification) :
    width_{specification.width},
    heigth_{specification.height},
    gl_format_(specification.texture_format == TextureFormat::kRgb ? GL_RGB : GL_RGBA) {
    GenerateTexture2D(data);
}

Texture2D::Texture2D(const ImageRgba& image) :
    Texture2D{image.GetRawImageData(), TextureSpecification{image.GetWidth(), image.GetHeight(), TextureFormat::kRgba}} {
}

Texture2D::Texture2D(const TextureSpecification& specification) :
    width_{specification.width},
    heigth_{specification.height},
    gl_format_(specification.texture_format == TextureFormat::kRgb ? GL_RGB : GL_RGBA) {
    GenerateTexture2D(nullptr);
}

Texture2D::~Texture2D() {
    int num_components = 3;

    if (GetGlFormat() == GL_RGBA) {
        num_components = 4;
    }

    num_texture_vram_used -= width_ * heigth_ * num_components;
    glDeleteTextures(1, &renderer_id_);
}

std::int32_t Texture2D::GetWidth() const {
    return width_;
}

std::int32_t Texture2D::GetHeight() const {
    return heigth_;
}

void Texture2D::Bind(uint32_t texture_unit) const {
    glBindTextureUnit(texture_unit, renderer_id_);
}

void Texture2D::Unbind(uint32_t texture_unit) {
    glBindTextureUnit(texture_unit, 0);
}

bool Texture2D::GotMinimaps() const {
    return has_mipmaps_;
}

void Texture2D::GenerateMipmaps() {
    glGenerateMipmap(renderer_id_);
    has_mipmaps_ = true;
}

TextureFormat Texture2D::GetTextureFormat() const {
    return gl_format_ == GL_RGB ? TextureFormat::kRgb : TextureFormat::kRgba;
}

void Texture2D::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset) {
    ERR_FAIL_EXPECTED_TRUE(specification.width < GetWidth() && specification.height < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(specification.width < GetWidth() && specification.height < GetHeight());

    glTextureSubImage2D(renderer_id_, 0, offset.x, offset.y,
        specification.width, specification.height, ConvertTextureFormatToGL(specification.texture_format), GL_UNSIGNED_BYTE, data);
}

uint32_t Texture2D::GetGlFormat() const {
    return gl_format_;
}

void Texture2D::GenerateTexture2D(const void* data) {
    glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);
    glBindTextureUnit(0, renderer_id_); // Binding to texture unit 0 by default

    SetStandardTextureOptions();

    glTextureStorage2D(renderer_id_, 1, GetGlFormat() == GL_RGB ? GL_RGB8 : GL_RGBA8, width_, heigth_);

    if (data != nullptr) {
        glTextureSubImage2D(renderer_id_, 0, 0, 0, width_, heigth_, GetGlFormat(), GL_UNSIGNED_BYTE, data);

        int num_components = 3;

        if (GetGlFormat() == GL_RGBA) {
            num_components = 4;
        }

        num_texture_vram_used += width_ * heigth_ * num_components;
    }

}

void Texture2D::SetStandardTextureOptions() {
    // Set texture wrapping and filtering options
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

ImageRgba LoadRgbaImageFromMemory(const void* data, int length) {
    stbi_image_data_t image_data = stbi_load_from_memory_rgba(reinterpret_cast<const uint8_t*>(data), length);
    return ImageRgba{image_data.data, image_data.width, image_data.height, &StbiDeleter};
}
