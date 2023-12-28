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

struct TextureSpecification
{
    int32_t Width;
    int32_t Height;
    TextureFormat Format;
};

class ITexture
{
public:
    virtual ~ITexture() = default;

public:
    virtual int32_t GetWidth() const = 0;
    virtual int32_t GetHeight() const = 0;

    virtual void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) = 0;
    virtual void Bind(int32_t textureUnit) const = 0;
    virtual void Unbind(int32_t textureUnit) = 0;

    virtual bool GotMinimaps() const = 0;
    virtual void GenerateMipmaps() = 0;

    virtual TextureFormat GetTextureFormat() const = 0;
};

class ITexture2D : public ITexture
{
public:
    static std::shared_ptr<ITexture2D> LoadFromFile(const std::string& filePath);
    static std::shared_ptr<ITexture2D> Create(const void* data, const TextureSpecification& specification);
    static std::shared_ptr<ITexture2D> CreateFromImage(const ImageRgba& image);
    static std::shared_ptr<ITexture2D> CreateEmpty(const TextureSpecification& specification);
};

ImageRgba LoadRgbaImageFromMemory(const void* data, int32_t length);