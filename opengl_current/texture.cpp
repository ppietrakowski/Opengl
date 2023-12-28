#include "Texture.h"

#include "error_macros.h"
#include "logging.h"
#include "renderer_api.h"
#include "opengl_texture.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION // Ensure that stb_image implementation is included in one source file
#define STBI_FAILURE_USERMSG

extern "C" {
#include "stb_image.h"
}

static void StbiDeleter(uint8_t* bytes)
{
    stbi_image_free(bytes);
}

std::shared_ptr<ITexture2D> ITexture2D::LoadFromFile(const std::string& filePath)
{
    if (IRendererAPI::GetApi() == IRendererAPI::kOpenGL)
    {
        // Flip the image vertically if needed
        stbi_set_flip_vertically_on_load(1);
    }
    else
    {
        stbi_set_flip_vertically_on_load(0);
    }

    int32_t width;
    int32_t height;
    int32_t channels;
    uint8_t* data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr)
    {
        throw std::runtime_error{"Failed to load texture " + filePath};
    }

    return CreateFromImage(ImageRgba(data, (uint32_t)width, (uint32_t)height, &StbiDeleter));
}

std::shared_ptr<ITexture2D> ITexture2D::Create(const void* data, uint32_t width, uint32_t height, TextureFormat format)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(data, width, height, format);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

std::shared_ptr<ITexture2D> ITexture2D::CreateFromImage(const ImageRgba& image)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(image);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

std::shared_ptr<ITexture2D> ITexture2D::CreateEmpty(uint32_t width, uint32_t height, TextureFormat format)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(width, height, format);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

ImageRgba LoadRgbaImageFromMemory(const void* data, uint32_t length)
{
    int32_t w, h, comps;
    uint8_t* image = stbi_load_from_memory((uint8_t*)data, length, &w, &h, &comps, STBI_rgb_alpha);
    return ImageRgba{image, (uint32_t)w, (uint32_t)h, &StbiDeleter};
}
