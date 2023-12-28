#pragma once

#include "texture.h"

class OpenGlTexture2D : public ITexture2D
{
public:
    OpenGlTexture2D(uint32_t width, uint32_t height, TextureFormat format);
    OpenGlTexture2D(const void* data, uint32_t width, uint32_t height, TextureFormat format);
    OpenGlTexture2D(const ImageRgba& image);
    ~OpenGlTexture2D();

    uint32_t GetWidth() const override;
    uint32_t GetHeight() const override;

    void SetData(const void* data, glm::uvec2 size, glm::uvec2 offset = {0, 0}) override;
    void Bind(uint32_t textureUnit) const override;
    void Unbind(uint32_t textureUnit) override;

    bool GotMinimaps() const override;
    void GenerateMipmaps() override;

    TextureFormat GetTextureFormat() const override;

private:
    uint32_t m_RendererId;
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_GlFormat;
    bool m_bGotMipmaps : 1;

private:
    void GenerateTexture2D(const void* data);
    void SetStandardTextureOptions();
    uint32_t GetGlFormat() const;
};

