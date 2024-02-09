#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include <filesystem>
#include <span>

#include "ImageRgba.hpp"

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
    virtual int32_t GetWidth() const = 0;
    virtual int32_t GetHeight() const = 0;

    virtual void Bind(uint32_t textureUnit) const = 0;
    virtual void Unbind(uint32_t textureUnit) = 0;

    virtual bool IsMipmapped() const = 0;
    virtual bool IsTranslucent() const = 0;
    virtual void GenerateMipmaps() = 0;
    virtual TextureFormat GetTextureFormat() const = 0;

    virtual uint32_t GetRendererId() const = 0;

    virtual void SetFilteringType(FilteringType filteringType) = 0;
    virtual const char* GetName() const = 0;
};

class Texture2D : public Texture
{
public:
    Texture2D(const std::filesystem::path& filePath);
    Texture2D(const void* data, const TextureSpecification& specification);
    Texture2D(const ImageRgba& image);
    Texture2D(const TextureSpecification& specification);
    virtual ~Texture2D();

public:
    virtual int32_t GetWidth() const override;
    virtual int32_t GetHeight() const override;

    virtual void Bind(uint32_t textureUnit) const override;
    virtual void Unbind(uint32_t textureUnit) override;

    virtual bool IsMipmapped() const override;
    virtual bool IsTranslucent() const override;

    virtual void GenerateMipmaps() override;

    virtual uint32_t GetRendererId() const override;

    virtual TextureFormat GetTextureFormat() const override;
    virtual void SetFilteringType(FilteringType filteringType) override;
    virtual const char* GetName() const override;

    static inline size_t s_NumTextureVramUsed = 0;

private:
    uint32_t m_RendererId;
    int32_t m_Width;
    int32_t m_Height;
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
    virtual int32_t GetWidth() const override;
    virtual int32_t GetHeight() const override;
    virtual void Bind(uint32_t textureUnit) const override;
    virtual void Unbind(uint32_t textureUnit) override;
    virtual bool IsMipmapped() const override;
    virtual bool IsTranslucent() const override;
    virtual void GenerateMipmaps() override;
    virtual TextureFormat GetTextureFormat() const override;
    virtual void SetFilteringType(FilteringType filteringType) override;

    virtual uint32_t GetRendererId() const override;
    virtual const char* GetName() const override;

private:
    uint32_t m_RendererId;
    std::string m_Name;
};

ImageRgba LoadRgbaImageFromMemory(const void* data, int32_t length);

inline uint32_t Texture2D::GetRendererId() const
{
    return m_RendererId;
}

inline uint32_t CubeMap::GetRendererId() const
{
    return m_RendererId;
}

inline const char* CubeMap::GetName() const
{
    return m_Name.c_str();
}
