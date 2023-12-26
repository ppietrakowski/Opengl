#include "opengl_texture.h"
#include "error_macros.h"

#include <GL/glew.h>

OpenGlTexture2D::OpenGlTexture2D(const void* data, uint32_t width, uint32_t height, TextureFormat format) :
    width_{ width },
    height_{ height },
    format_(format == TextureFormat::kRgb ? GL_RGB : GL_RGBA) {
    GenerateTexture2D(data);
}

OpenGlTexture2D::OpenGlTexture2D(const ImageRgba& image) :
    OpenGlTexture2D{ image.GetRawImageData(), image.GetWidth(), image.GetHeight(), TextureFormat::kRgba } {}

OpenGlTexture2D::OpenGlTexture2D(uint32_t width, uint32_t height, TextureFormat format) :
    width_{ width },
    height_{ height },
    format_(format == TextureFormat::kRgb ? GL_RGB : GL_RGBA) {
    GenerateTexture2D(nullptr);
}

OpenGlTexture2D::~OpenGlTexture2D() {
    glDeleteTextures(1, &renderer_id_);
}

uint32_t OpenGlTexture2D::GetWidth() const {
    return width_;
}

uint32_t OpenGlTexture2D::GetHeight() const {
    return height_;
}

void OpenGlTexture2D::Bind(uint32_t texture_unit) const {
    glBindTextureUnit(texture_unit, renderer_id_);
}

void OpenGlTexture2D::Unbind(uint32_t texture_unit) {
    glBindTextureUnit(texture_unit, 0);
}

bool OpenGlTexture2D::GotMinimaps() const {
    return got_mipmaps_;
}

void OpenGlTexture2D::GenerateMipmaps() {
    glGenerateMipmap(renderer_id_);
    got_mipmaps_ = true;
}

TextureFormat OpenGlTexture2D::GetTextureFormat() const {
    return format_ == GL_RGB ? TextureFormat::kRgb : TextureFormat::kRgba;
}

void OpenGlTexture2D::SetData(const void* data, glm::uvec2 size, glm::uvec2 offset) {
    ERR_FAIL_EXPECTED_TRUE(size.x < GetWidth() && size.y < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(offset.x < GetWidth() && offset.y < GetHeight());

    glTextureSubImage2D(renderer_id_, 0, offset.x, offset.y, size.x, size.y, GetGlFormat(), GL_UNSIGNED_BYTE, data);
}

uint32_t OpenGlTexture2D::GetGlFormat() const {
    return format_;
}

void OpenGlTexture2D::GenerateTexture2D(const void* data) {
    glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id_);
    glBindTextureUnit(0, renderer_id_); // Binding to texture unit 0 by default

    SetStandardTextureOptions();

    glTextureStorage2D(renderer_id_, 1, GetGlFormat() == GL_RGB ? GL_RGB8 : GL_RGBA8, width_, height_);

    if (data != nullptr) {
        glTextureSubImage2D(renderer_id_, 0, 0, 0, width_, height_, GetGlFormat(), GL_UNSIGNED_BYTE, data);
    }
}

void OpenGlTexture2D::SetStandardTextureOptions() {
    // Set texture wrapping and filtering options
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(renderer_id_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(renderer_id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

#if 0
void OpenGlTexture2D::LoadFromFile(const std::string& file_path) {
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

#endif