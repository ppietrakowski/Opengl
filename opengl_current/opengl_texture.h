#pragma once

#include "texture.h"

class OpenGlTexture2D : public ITexture2D
{
public:
    OpenGlTexture2D(const TextureSpecification& specification);
    OpenGlTexture2D(const void* data, const TextureSpecification& specification);
    OpenGlTexture2D(const ImageRgba& image);
    ~OpenGlTexture2D();

    int32_t GetWidth() const override;
    int32_t GetHeight() const override;

    void SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset = {0, 0}) override;
    void Bind(int32_t textureUnit) const override;
    void Unbind(int32_t textureUnit) override;

    bool GotMinimaps() const override;
    void GenerateMipmaps() override;

    TextureFormat GetTextureFormat() const override;

private:
    uint32_t m_RendererId;
    int32_t m_Width;
    int32_t m_Height;
    uint32_t m_GlFormat;
    bool m_bGotMipmaps : 1;

private:
    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
    uint32_t GetGlFormat() const;
};

