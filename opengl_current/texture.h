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
    Rgb,
    Rgba
};

struct TextureSpecification
{
    std::int32_t Width;
    std::int32_t Height;
    TextureFormat Format;
};

class Texture
{
public:
    virtual ~Texture() = default;

public:
    virtual std::int32_t GetWidth() const = 0;
    virtual std::int32_t GetHeight() const = 0;

    virtual void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) = 0;
    virtual void Bind(std::uint32_t textureUnit) const = 0;
    virtual void Unbind(std::uint32_t textureUnit) = 0;

    virtual bool GotMinimaps() const = 0;
    virtual void GenerateMipmaps() = 0;

    virtual TextureFormat GetTextureFormat() const = 0;
};

class Texture2D : public Texture
{
public:
    static std::shared_ptr<Texture2D> LoadFromFile(const std::filesystem::path& filePath);
    static std::shared_ptr<Texture2D> Create(const void* data, const TextureSpecification& specification);
    static std::shared_ptr<Texture2D> CreateFromImage(const ImageRgba& image);
    static std::shared_ptr<Texture2D> CreateEmpty(const TextureSpecification& specification);
};

ImageRgba LoadRgbaImageFromMemory(const void* data, std::int32_t length);