#pragma once

#include <GL/glew.h>

#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include <filesystem>
#include <span>

enum class ETextureFormat
{
    Rgb,
    Rgba
};

class Texture
{
public:
    virtual ~Texture();

public:
    unsigned int GetWidth() const { return _width; }
    unsigned int GetHeight() const { return _height; }

    GLuint GetRendererID() const { return _rendererID; }

    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = { 0, 0 }) = 0;
    void Bind(unsigned int textureUnit) const;
    static void Unbind(unsigned int textureUnit);

    bool GotMinimaps() const { return _gotMinimaps; }

    void GenerateMipmaps();

    ETextureFormat GetTextureFormat() const;

protected:
    GLuint _rendererID;
    
protected:
    Texture(unsigned int width, unsigned int height, GLenum format);
    void OverrideOptions(unsigned int width, unsigned int height, GLenum format);

protected:
    GLenum GetGlFormat() const;

    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();

private:
    unsigned int _width;
    unsigned int _height;
    GLenum _format;
    bool _gotMinimaps : 1;
};

class Texture2D : public Texture
{
public:
    Texture2D(const std::string& filepath);
    Texture2D(const void* data, unsigned int width, unsigned int height, ETextureFormat format);
    Texture2D(unsigned int width, unsigned int height, ETextureFormat format);

public:
    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = { 0, 0 }) override;

private:
    void LoadFromFile(const std::string& filepath);
};

