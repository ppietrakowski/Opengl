#include "opengl_texture.h"
#include "error_macros.h"

#include <GL/glew.h>

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

OpenGlTexture2D::OpenGlTexture2D(const void* data, const TextureSpecification& specification) :
    width_{specification.width},
    height_{specification.height},
    gl_format_(specification.texture_format == TextureFormat::kRgb ? GL_RGB : GL_RGBA) {
    GenerateTexture2D(data);
}

OpenGlTexture2D::OpenGlTexture2D(const ImageRgba& image) :
    OpenGlTexture2D{image.GetRawImageData(), TextureSpecification{image.GetWidth(), image.GetHeight(), TextureFormat::kRgba}} {
}

OpenGlTexture2D::OpenGlTexture2D(const TextureSpecification& specification) :
    width_{specification.width},
    height_{specification.height},
    gl_format_(specification.texture_format == TextureFormat::kRgb ? GL_RGB : GL_RGBA) {
    GenerateTexture2D(nullptr);
}

OpenGlTexture2D::~OpenGlTexture2D() {
    glDeleteTextures(1, &renderer_id_);
}

std::int32_t OpenGlTexture2D::GetWidth() const {
    return width_;
}

std::int32_t OpenGlTexture2D::GetHeight() const {
    return height_;
}

void OpenGlTexture2D::Bind(std::uint32_t texture_unit) const {
    glBindTextureUnit(texture_unit, renderer_id_);
}

void OpenGlTexture2D::Unbind(std::uint32_t texture_unit) {
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
    return gl_format_ == GL_RGB ? TextureFormat::kRgb : TextureFormat::kRgba;
}

void OpenGlTexture2D::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset) {
    ERR_FAIL_EXPECTED_TRUE(specification.width < GetWidth() && specification.height < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(specification.width < GetWidth() && specification.height < GetHeight());

    glTextureSubImage2D(renderer_id_, 0, offset.x, offset.y,
        specification.width, specification.height, ConvertTextureFormatToGL(specification.texture_format), GL_UNSIGNED_BYTE, data);
}

uint32_t OpenGlTexture2D::GetGlFormat() const {
    return gl_format_;
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