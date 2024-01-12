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
    Texture2D(const std::filesystem::path& filePath);
    Texture2D(const void* data, const TextureSpecification& specification);
    Texture2D(const ImageRgba& image);
    Texture2D(const TextureSpecification& specification);
    ~Texture2D();

public:
    std::int32_t GetWidth() const override;
    std::int32_t GetHeight() const override;

    void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) override;
    void Bind(std::uint32_t textureUnit) const override;
    void Unbind(std::uint32_t textureUnit) override;

    bool GotMinimaps() const override;
    void GenerateMipmaps() override;

    TextureFormat GetTextureFormat() const override;

private:
    std::uint32_t m_RendererId;
    std::int32_t m_Width;
    std::int32_t m_Height;
    std::uint32_t m_GlFormat;
    bool m_bHasMipmaps : 1{false};

private:
    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
    uint32_t GetGlFormat() const;
};

ImageRgba LoadRgbaImageFromMemory(const void* data, std::int32_t length);