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
    load_path_ = file_path.string();
    
    std::string path = file_path.string();
    
    int num_comps{0};

    uint8_t *data = stbi_load(path.c_str(), &width_, &heigth_, &num_comps, STBI_rgb_alpha);


    if (data == nullptr) {
        throw std::runtime_error{"Failed to load texture " + path};
    }

    internal_data_format_ = GL_RGBA8;
    data_format_ = GL_RGBA;

    GenerateTexture2D(data);

    STBI_FREE(data);
}

Texture2D::Texture2D(const void* data, const TextureSpecification& specification) :
    width_{specification.width},
    heigth_{specification.height} {

    if (specification.texture_format == TextureFormat::kRgba) {
        internal_data_format_ = GL_RGBA8;
        data_format_ = GL_RGBA;
    } else if (specification.texture_format == TextureFormat::kRgb) {
        internal_data_format_ = GL_RGB8;
        data_format_ = GL_RGB;
    }

    GenerateTexture2D(data);
}

Texture2D::Texture2D(const ImageRgba& image) :
    Texture2D{image.GetRawImageData(), TextureSpecification{image.GetWidth(), image.GetHeight(), TextureFormat::kRgba}} {
}

Texture2D::Texture2D(const TextureSpecification& specification) :
    width_{specification.width},
    heigth_{specification.height} {

    if (specification.texture_format == TextureFormat::kRgba) {
        internal_data_format_ = GL_RGBA8;
        data_format_ = GL_RGBA;
    } else if (specification.texture_format == TextureFormat::kRgb) {
        internal_data_format_ = GL_RGB8;
        data_format_ = GL_RGB;
    }
    GenerateTexture2D(nullptr);
}

Texture2D::~Texture2D() {
    int num_components = 3;

    if (data_format_ == GL_RGBA) {
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
    return data_format_ == GL_RGB ? TextureFormat::kRgb : TextureFormat::kRgba;
}

void Texture2D::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset) {
    ERR_FAIL_EXPECTED_TRUE(specification.width < GetWidth() && specification.height < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(specification.width < GetWidth() && specification.height < GetHeight());

    glTextureSubImage2D(renderer_id_, 0, offset.x, offset.y,
        specification.width, specification.height, ConvertTextureFormatToGL(specification.texture_format), GL_UNSIGNED_BYTE, data);
}

void Texture2D::GenerateTexture2D(const void* data) {
    glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);

    SetStandardTextureOptions();

    glTextureStorage2D(renderer_id_, 1, internal_data_format_, width_, heigth_);

    if (data != nullptr) {
        glTextureSubImage2D(renderer_id_, 0, 0, 0, width_, heigth_, data_format_, GL_UNSIGNED_BYTE, data);

        int num_components = 3;

        if (data_format_ == GL_RGBA) {
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
    RgbColor* rgb_color = (RgbColor*)image_data.data;

    uint8_t* cls = new uint8_t[sizeof(RgbaColor) * image_data.width * image_data.height];;
    RgbaColor* color = (RgbaColor*)cls;

    for (int x = 0; x < image_data.width; ++x) {
        for (int y = 0; y < image_data.height; ++y) {
            const RgbColor& c = rgb_color[x + y * image_data.width];

            color[x + y * image_data.width] = RgbaColor{c.red, c.green, c.blue, 255};
        }
    }

    STBI_FREE(rgb_color);

    return ImageRgba{cls, image_data.width, image_data.height};
}

CubeMap::CubeMap(std::span<const std::string> paths) {
    // Flip the image vertically if needed
    stbi_set_flip_vertically_on_load(0);
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &renderer_id_);
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTextureUnit(0, renderer_id_);

    for (uint32_t i = 0; i < paths.size(); ++i) {
        const std::string& path = paths[i];
        stbi_image_data_t image_data = stbi_load_from_filepath(path.c_str(), STBI_rgb_alpha);

        if (image_data.data == nullptr) {
            throw std::runtime_error{"Failed to load texture " + path};
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGBA, image_data.width, image_data.height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.data
        );

        Texture2D::num_texture_vram_used += 4 * image_data.width * image_data.height;

        STBI_FREE(image_data.data);
    }
}

CubeMap::~CubeMap() {
    glDeleteTextures(1, &renderer_id_);
}

int CubeMap::GetWidth() const {
    return 0;
}

int CubeMap::GetHeight() const {
    return 0;
}

void CubeMap::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset) {
}

void CubeMap::Bind(uint32_t texture_unit) const {
    glBindTextureUnit(texture_unit, renderer_id_);
}

void CubeMap::Unbind(uint32_t texture_unit) {
    glBindTextureUnit(texture_unit, 0);
}

bool CubeMap::GotMinimaps() const {
    return false;
}

void CubeMap::GenerateMipmaps() {
}

TextureFormat CubeMap::GetTextureFormat() const {
    return TextureFormat::kRgba;
}
