#include "image_rgba.h"

#include <algorithm>
#include <cstring>

ImageRgba::ImageRgba(uint8_t* image, int32_t width, int32_t height, void(*deleter)(uint8_t*)) :
    ImageData{image},
    Width{width},
    Height{height},
    Deleter{deleter}
{
}

ImageRgba::ImageRgba(ImageRgba&& image) noexcept
{
    *this = std::move(image);
}

ImageRgba& ImageRgba::operator=(ImageRgba&& image) noexcept
{
    ImageData = image.ImageData;
    Width = image.Width;
    Height = image.Height;
    Deleter = image.Deleter;
    memset(&image, 0, sizeof(image));

    return *this;
}

ImageRgba::~ImageRgba()
{
    Deleter(ImageData);
}

const uint8_t* ImageRgba::GetRawImageData() const
{
    return ImageData;
}

int32_t ImageRgba::GetWidth() const
{
    return Width;
}

int32_t ImageRgba::GetHeight() const
{
    return Height;
}
