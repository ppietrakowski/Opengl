#include "image_rgba.h"

#include <algorithm>
#include <cstring>

ImageRgba::ImageRgba(std::uint8_t* image, std::int32_t Width, std::int32_t height, void(*deleter)(std::uint8_t*)) :
    m_ImageData{image},
    m_Width{Width},
    m_Height{height},
    m_Deleter{deleter}
{
}

ImageRgba::ImageRgba(ImageRgba&& image) noexcept
{
    *this = std::move(image);
}

ImageRgba& ImageRgba::operator=(ImageRgba&& image) noexcept
{
    m_ImageData = image.m_ImageData;
    m_Width = image.m_Width;
    m_Height = image.m_Height;
    m_Deleter = image.m_Deleter;
    memset(&image, 0, sizeof(image));

    return *this;
}

ImageRgba::~ImageRgba()
{
    m_Deleter(m_ImageData);
}

const std::uint8_t* ImageRgba::GetRawImageData() const
{
    return m_ImageData;
}

std::int32_t ImageRgba::GetWidth() const
{
    return m_Width;
}

std::int32_t ImageRgba::GetHeight() const
{
    return m_Height;
}
