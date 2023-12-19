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
    Rgb,
    Rgba
};

class Texture
{
public:
    virtual ~Texture();

public:
    std::uint32_t GetWidth() const { return _width; }
    std::uint32_t GetHeight() const { return _height; }

    GLuint GetRendererID() const { return _rendererID; }

    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = { 0, 0 }) = 0;
    void Bind(std::uint32_t TextureUnit) const;
    static void Unbind(std::uint32_t TextureUnit);

    bool GotMinimaps() const { return _gotMinimaps; }

    void GenerateMipmaps();

    TextureFormat GetTextureFormat() const;

protected:
    GLuint _rendererID;
    std::uint32_t _width;
    std::uint32_t _height;
    GLenum _format;
    bool _gotMinimaps : 1;

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
    Texture2D(const std::string& filepath);
    Texture2D(const void* data, std::uint32_t width, std::uint32_t height, TextureFormat format);
    Texture2D(std::uint32_t width, std::uint32_t height, TextureFormat format);

public:
    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = { 0, 0 }) override;

private:
    void LoadFromFile(const std::string& filepath);
};

