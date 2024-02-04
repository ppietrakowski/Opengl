#include "image_rgba.h"

#include <algorithm>
#include <cstring>

ImageRgba::ImageRgba(uint8_t* image, int32_t width, int32_t height, void(*deleter)(uint8_t*)) :
    m_ImageData{image, DefaultImageDeleter{deleter}},
    m_Width{width},
    m_Height{height}
{
}

ImageRgba::ImageRgba(ImageRgba&& image) noexcept
{
    *this = std::move(image);
}

ImageRgba& ImageRgba::operator=(ImageRgba&& image) noexcept
{
    m_ImageData = std::move(image.m_ImageData);
    m_Width = image.m_Width;
    m_Height = image.m_Height;
    memset(&image, 0, sizeof(image));

    return *this;
}

const uint8_t* ImageRgba::GetRawImageData() const
{
    return m_ImageData.get();
}

int32_t ImageRgba::GetWidth() const
{
    return m_Width;
}

int32_t ImageRgba::GetHeight() const
{
    return m_Height;
}
