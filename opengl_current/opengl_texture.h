#pragma once

#include "texture.h"

class OpenGlTexture2D : public Texture2D
{
public:
    OpenGlTexture2D(const TextureSpecification& specification);
    OpenGlTexture2D(const void* data, const TextureSpecification& specification);
    OpenGlTexture2D(const ImageRgba& image);
    ~OpenGlTexture2D();

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
    bool m_bHasMipmaps : 1;

private:
    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
    uint32_t GetGlFormat() const;
};

