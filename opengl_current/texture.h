#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include <filesystem>
#include <span>

#include "image_rgba.h"

enum class TextureFormat
{
    kRgb,
    kRgba
};

class ITexture
{
public:
    virtual ~ITexture() = default;

public:
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = {0, 0}) = 0;
    virtual void Bind(uint32_t textureUnit) const = 0;
    virtual void Unbind(uint32_t textureUnit) = 0;

    virtual bool GotMinimaps() const = 0;
    virtual void GenerateMipmaps() = 0;

    virtual TextureFormat GetTextureFormat() const = 0;
};

class ITexture2D : public ITexture
{
public:
    static std::shared_ptr<ITexture2D> LoadFromFile(const std::string& filePath);
    static std::shared_ptr<ITexture2D> Create(const void* data, uint32_t width, uint32_t height, TextureFormat format);
    static std::shared_ptr<ITexture2D> CreateFromImage(const ImageRgba& image);
    static std::shared_ptr<ITexture2D> CreateEmpty(uint32_t width, uint32_t height, TextureFormat format);
};

ImageRgba LoadRgbaImageFromMemory(const void* data, uint32_t length);