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
    int Width;
    int Height;
    TextureFormat Format;
};

enum class FilteringType : uint8_t
{
    Linear = 0,
    Nearest
};

class Texture
{
public:
    virtual ~Texture() = default;

public:
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    virtual void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) = 0;
    virtual void Bind(uint32_t textureUnit) const = 0;
    virtual void Unbind(uint32_t textureUnit) = 0;

    virtual bool GotMinimaps() const = 0;
    virtual void GenerateMipmaps() = 0;
    virtual TextureFormat GetTextureFormat() const = 0;

    virtual const void* GetRendererId() const = 0;

    virtual void SetFilteringType(FilteringType filteringType) = 0;
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
    void Bind(uint32_t textureUnit) const override;
    void Unbind(uint32_t textureUnit) override;

    bool GotMinimaps() const override;
    void GenerateMipmaps() override;


    const void* GetRendererId() const override
    {
        return (const void*)static_cast<intptr_t>(m_RendererId);
    }

    TextureFormat GetTextureFormat() const override;
    void SetFilteringType(FilteringType filteringType) override;

    static inline size_t s_NumTextureVramUsed = 0;


private:
    uint32_t m_RendererId;
    int m_Width;
    int m_Height;
    uint32_t m_DataFormat{0};
    uint32_t m_InternalDataFormat{0};
    bool m_bHasMipmaps : 1{false};
    std::string m_LoadPath;

private:
    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
};


struct CubeMapTextureIndex
{
    enum Index
    {
        Back,
        Bottom,
        Front,
        Left,
        Right,
        Top,
        Count
    };
};

class CubeMap : public Texture
{
public:
    CubeMap(std::span<const std::string> paths);
    ~CubeMap();

public:
    // Inherited via Texture
    int GetWidth() const override;
    int GetHeight() const override;
    void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset) override;
    void Bind(uint32_t textureUnit) const override;
    void Unbind(uint32_t textureUnit) override;
    bool GotMinimaps() const override;
    void GenerateMipmaps() override;
    TextureFormat GetTextureFormat() const override;
    void SetFilteringType(FilteringType filteringType) override;

    const void* GetRendererId() const override
    {
        return (const void*)static_cast<intptr_t>(m_RendererId);
    }

private:
    uint32_t m_RendererId;
};

ImageRgba LoadRgbaImageFromMemory(const void* data, int length);