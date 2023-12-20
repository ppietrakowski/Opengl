#pragma once

#include <GL/glew.h>

#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include <filesystem>
#include <span>

extern "C" {
#include "stb_image.h"
}

enum class TextureFormat
{
    kRgb,
    kRgba
};

class Texture
{
public:
    virtual ~Texture();

public:
    std::uint32_t GetWidth() const { return width_; }
    std::uint32_t GetHeight() const { return height_; }

    GLuint GetRendererID() const { return renderer_id_; }

    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = { 0, 0 }) = 0;
    void Bind(std::uint32_t texture_unit) const;
    static void Unbind(std::uint32_t texture_unit);

    bool GotMinimaps() const { return got_mipmaps_; }

    void GenerateMipmaps();

    TextureFormat GetTextureFormat() const;

protected:
    GLuint renderer_id_;
    std::uint32_t width_;
    std::uint32_t height_;
    GLenum format_;
    bool got_mipmaps_ : 1;

protected:
    Texture(std::uint32_t width, std::uint32_t height, GLenum format);

protected:
    GLenum GetGlFormat() const;

    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
};

class Texture2D : public Texture
{
public:
    Texture2D(const std::string& file_path);
    Texture2D(const void* data, std::uint32_t width, std::uint32_t height, TextureFormat format);
    Texture2D(std::uint32_t width, std::uint32_t height, TextureFormat format);

public:
    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = { 0, 0 }) override;

private:
    void LoadFromFile(const std::string& file_path);
};

