#include "Texture.h"

#include "error_macros.h"
#include "logging.h"
#include "renderer_api.h"

#include <GL/glew.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION // Ensure that stb_image implementation is included in one source file
#define STBI_FAILURE_USERMSG

extern "C" {
#include "stb_image.h"
}

static void StbiDeleteFunc(uint8_t* bytes)
{
    if (bytes != nullptr)
    {
        stbi_image_free(bytes);
    }
}

struct StbiDeleter
{
    void operator()(uint8_t* bytes)
    {
        if (bytes != nullptr)
        {
            stbi_image_free(bytes);
        }
    }
};

using StbiImageUniquePtr = std::unique_ptr<uint8_t, StbiDeleter>;

struct StbiImageData
{
    int Width{0};
    int Height{0};
    int NumComps{0};

    bool IsRgb() const
    {
        return NumComps == 3;
    }

    int NumPixels() const
    {
        return Width * Height;
    }

    GLenum GetDataFormat() const
    {
        ASSERT(NumComps >= 3 && NumComps <= 4);
        return NumComps == 3 ? GL_RGB : GL_RGBA;
    }

    GLenum GetInternalFormat() const
    {
        ASSERT(NumComps >= 3 && NumComps <= 4);
        return NumComps == 3 ? GL_RGB8 : GL_RGBA8;
    }
};

static GLenum ConvertTextureFormatToDataFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::Rgb:
        return GL_RGB;
    case TextureFormat::Rgba:
        return GL_RGBA;
    }

    return 0;
}

static GLenum ConvertTextureFormatToInternalFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::Rgb:
        return GL_RGB8;
    case TextureFormat::Rgba:
        return GL_RGBA8;
    }

    return 0;
}

static StbiImageUniquePtr MakeImageUniquePtrFromMemory(const void* data, int length, StbiImageData& outImageData);

static StbiImageUniquePtr LoadImageFromFilePath(const std::string& filePath, StbiImageData& outImageData)
{
    uint8_t* image = stbi_load(filePath.c_str(), &outImageData.Width, &outImageData.Height,
        &outImageData.NumComps, 0);

    if (image == nullptr)
    {
        throw std::runtime_error{"Failed to load texture " + std::string{stbi_failure_reason()}};
    }

    return StbiImageUniquePtr(image);
}

Texture2D::Texture2D(const std::filesystem::path& filePath)
{
    // Flip the image vertically if needed
    stbi_set_flip_vertically_on_load(1);
    m_LoadPath = filePath.string();

    std::string path = filePath.string();

    StbiImageData imageData;
    StbiImageUniquePtr data = LoadImageFromFilePath(path, imageData);

    m_Width = imageData.Width;
    m_Height = imageData.Height;

    m_InternalDataFormat = imageData.GetInternalFormat();
    m_DataFormat = imageData.GetDataFormat();

    GenerateTexture2D(data.get());
}

Texture2D::Texture2D(const void* data, const TextureSpecification& specification) :
    m_Width{specification.Width},
    m_Height{specification.Height}
{

    m_InternalDataFormat = ConvertTextureFormatToInternalFormat(specification.Format);
    m_DataFormat = ConvertTextureFormatToDataFormat(specification.Format);

    GenerateTexture2D(data);
}

Texture2D::Texture2D(const ImageRgba& image) :
    Texture2D{image.GetRawImageData(), TextureSpecification{image.GetWidth(), image.GetHeight(), TextureFormat::Rgba}}
{
}

Texture2D::Texture2D(const TextureSpecification& specification) :
    Texture2D(nullptr, specification)
{
}

Texture2D::~Texture2D()
{
    int numComponents = 3;

    if (m_DataFormat == GL_RGBA)
    {
        numComponents = 4;
    }

    s_NumTextureVramUsed -= m_Width * m_Height * numComponents;
    glDeleteTextures(1, &m_RendererId);
}

std::int32_t Texture2D::GetWidth() const
{
    return m_Width;
}

std::int32_t Texture2D::GetHeight() const
{
    return m_Height;
}

void Texture2D::Bind(uint32_t textureUnit) const
{
    glBindTextureUnit(textureUnit, m_RendererId);
}

void Texture2D::Unbind(uint32_t textureUnit)
{
    glBindTextureUnit(textureUnit, 0);
}

bool Texture2D::GotMinimaps() const
{
    return m_bHasMipmaps;
}

void Texture2D::GenerateMipmaps()
{
    glGenerateMipmap(m_RendererId);
    m_bHasMipmaps = true;
}

TextureFormat Texture2D::GetTextureFormat() const
{
    return m_DataFormat == GL_RGB ? TextureFormat::Rgb : TextureFormat::Rgba;
}

static GLenum FilteringTypes[] = {GL_LINEAR, GL_NEAREST};

void Texture2D::SetFilteringType(FilteringType filteringType)
{
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, FilteringTypes[(size_t)filteringType]);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, FilteringTypes[(size_t)filteringType]);
}

void Texture2D::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset)
{
    ERR_FAIL_EXPECTED_TRUE(specification.Width < GetWidth() && specification.Height < GetHeight());
    ERR_FAIL_EXPECTED_TRUE(specification.Width < GetWidth() && specification.Height < GetHeight());

    glTextureSubImage2D(m_RendererId, 0, offset.x, offset.y,
        specification.Width, specification.Height, ConvertTextureFormatToDataFormat(specification.Format), GL_UNSIGNED_BYTE, data);
}

void Texture2D::GenerateTexture2D(const void* data)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);

    SetStandardTextureOptions();

    glTextureStorage2D(m_RendererId, 1, m_InternalDataFormat, m_Width, m_Height);

    if (data != nullptr)
    {
        glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

        int numComponents = 3;

        if (m_DataFormat == GL_RGBA)
        {
            numComponents = 4;
        }

        s_NumTextureVramUsed += m_Width * m_Height * numComponents;
    }
}

void Texture2D::SetStandardTextureOptions()
{
    // Set texture wrapping and filtering options
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static StbiImageUniquePtr ConvertRgbToRgbaImage(const uint8_t* rgbImage, const StbiImageData& imageData)
{
    ASSERT(rgbImage);

    const RgbColor* rgbColor = reinterpret_cast<const RgbColor*>(rgbImage);
    size_t length = sizeof(RgbaColor) * imageData.NumPixels();
    RgbaColor* imgData = reinterpret_cast<RgbaColor*>(STBI_MALLOC(length));

    if (imgData == nullptr)
    {
        throw std::runtime_error("Cannot convert image to RGBA out of memory");
    }

    for (int x = 0; x < imageData.Width; ++x)
    {
        for (int y = 0; y < imageData.Height; ++y)
        {
            int index = x + y * imageData.Width;

            const RgbColor& c = rgbColor[index];
            imgData[index] = RgbaColor(c.Red, c.Green, c.Blue);
        }
    }

    return StbiImageUniquePtr(reinterpret_cast<uint8_t*>(imgData));
}

StbiImageUniquePtr MakeImageUniquePtrFromMemory(const void* data, int length, StbiImageData& outImageData)
{
    ASSERT(data);

    uint8_t* imageData = stbi_load_from_memory(
        reinterpret_cast<const uint8_t*>(data), length, &outImageData.Width,
        &outImageData.Height, &outImageData.NumComps, 0);

    if (!imageData)
    {
        throw std::runtime_error(stbi_failure_reason());
    }

    return StbiImageUniquePtr(imageData);
}

ImageRgba LoadRgbaImageFromMemory(const void* data, int length)
{
    StbiImageData imageData;
    StbiImageUniquePtr img = MakeImageUniquePtrFromMemory(data, length, imageData);

    if (imageData.IsRgb())
    {
        img = ConvertRgbToRgbaImage(img.get(), imageData);
    }

    return ImageRgba{img.release(), imageData.Width, imageData.Height, &StbiDeleteFunc};
}

CubeMap::CubeMap(std::span<const std::string> paths)
{
    // Flip the image vertically if needed
    stbi_set_flip_vertically_on_load(0);
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererId);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTextureUnit(0, m_RendererId);

    for (uint32_t i = 0; i < paths.size(); ++i)
    {
        const std::string& path = paths[i];
        StbiImageData imageData;

        StbiImageUniquePtr image = LoadImageFromFilePath(path, imageData);

        GLenum internalDataFormat = imageData.GetInternalFormat();
        GLenum dataFormat = imageData.GetDataFormat();

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, dataFormat, imageData.Width, imageData.Height,
            0, dataFormat, GL_UNSIGNED_BYTE, image.get()
        );

        int memory_used = imageData.NumComps * imageData.NumPixels();
        Texture2D::s_NumTextureVramUsed += memory_used;
    }
}

CubeMap::~CubeMap()
{
    glDeleteTextures(1, &m_RendererId);
}

int CubeMap::GetWidth() const
{
    return 0;
}

int CubeMap::GetHeight() const
{
    return 0;
}

void CubeMap::SetData(const void* data, const TextureSpecification& specification, glm::ivec2 offset)
{
}

void CubeMap::Bind(uint32_t textureUnit) const
{
    glBindTextureUnit(textureUnit, m_RendererId);
}

void CubeMap::Unbind(uint32_t textureUnit)
{
    glBindTextureUnit(textureUnit, 0);
}

bool CubeMap::GotMinimaps() const
{
    return false;
}

void CubeMap::GenerateMipmaps()
{
}

TextureFormat CubeMap::GetTextureFormat() const
{
    return TextureFormat::Rgba;
}

void CubeMap::SetFilteringType(FilteringType filteringType)
{
}
