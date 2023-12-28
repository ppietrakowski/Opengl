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

    stbi_image_data_t imageData = stbi_load_from_filepath(filePath.c_str(), STBI_rgb_alpha);

    if (imageData.data == nullptr)
    {
        throw std::runtime_error{"Failed to load texture " + filePath};
    }

    return CreateFromImage(ImageRgba(imageData.data, imageData.width, imageData.height, &StbiDeleter));
}

std::shared_ptr<ITexture2D> ITexture2D::Create(const void* data, const TextureSpecification& specification)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(data, specification);
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

std::shared_ptr<ITexture2D> ITexture2D::CreateEmpty(const TextureSpecification& specification)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(specification);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

ImageRgba LoadRgbaImageFromMemory(const void* data, int length)
{
    stbi_image_data_t imageData = stbi_load_from_memory_rgba(reinterpret_cast<const uint8_t*>(data), length);
    return ImageRgba{imageData.data, imageData.width, imageData.height, &StbiDeleter};
}
