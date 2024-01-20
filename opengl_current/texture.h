#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include <filesystem>
#include <span>

#include "image_rgba.h"

enum class TextureFormat {
    kRgb,
    kRgba
};

struct TextureSpecification {
    int width;
    int height;
    TextureFormat texture_format;
};

class Texture {
public:
    virtual ~Texture() = default;

public:
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    virtual void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) = 0;
    virtual void Bind(uint32_t texture_unit) const = 0;
    virtual void Unbind(uint32_t texture_unit) = 0;

    virtual bool GotMinimaps() const = 0;
    virtual void GenerateMipmaps() = 0;

    virtual TextureFormat GetTextureFormat() const = 0;
};

class Texture2D : public Texture {
public:
    Texture2D(const std::filesystem::path& file_path);
    Texture2D(const void* data, const TextureSpecification& specification);
    Texture2D(const ImageRgba& image);
    Texture2D(const TextureSpecification& specification);
    ~Texture2D();

public:
    std::int32_t GetWidth() const override;
    std::int32_t GetHeight() const override;

    void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) override;
    void Bind(uint32_t texture_unit) const override;
    void Unbind(uint32_t texture_unit) override;

    bool GotMinimaps() const override;
    void GenerateMipmaps() override;

    TextureFormat GetTextureFormat() const override;

    static inline size_t num_texture_vram_used = 0;

private:
    uint32_t renderer_id_;
    int width_;
    int heigth_;
    uint32_t gl_format_;
    bool has_mipmaps_ : 1{false};

private:
    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
    uint32_t GetGlFormat() const;
};

ImageRgba LoadRgbaImageFromMemory(const void* data, int length);