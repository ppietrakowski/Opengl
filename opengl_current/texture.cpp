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

static void StbiDeleter(std::uint8_t* bytes) {
    stbi_image_free(bytes);
}

std::shared_ptr<Texture2D> Texture2D::LoadFromFile(const std::filesystem::path& file_path) {
    if (RendererAPI::GetApi() == RendererAPI::kOpenGL) {
        // Flip the image vertically if needed
        stbi_set_flip_vertically_on_load(1);
    } else {
        stbi_set_flip_vertically_on_load(0);
    }

    std::string path = file_path.string();
    stbi_image_data_t imageData = stbi_load_from_filepath(path.c_str(), STBI_rgb_alpha);

    if (imageData.data == nullptr) {
        throw std::runtime_error{"Failed to load texture " + path};
    }

    return CreateFromImage(ImageRgba(imageData.data, imageData.width, imageData.height, &StbiDeleter));
}

std::shared_ptr<Texture2D> Texture2D::Create(const void* data, const TextureSpecification& specification) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(data, specification);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

std::shared_ptr<Texture2D> Texture2D::CreateFromImage(const ImageRgba& image) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(image);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

std::shared_ptr<Texture2D> Texture2D::CreateEmpty(const TextureSpecification& specification) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlTexture2D>(specification);
    }

    ERR_FAIL_MSG_V("Invalid API type", nullptr);
}

ImageRgba LoadRgbaImageFromMemory(const void* data, std::int32_t length) {
    stbi_image_data_t imageData = stbi_load_from_memory_rgba(reinterpret_cast<const std::uint8_t*>(data), length);
    return ImageRgba{imageData.data, imageData.width, imageData.height, &StbiDeleter};
}
