#include "ImageRgba.hpp"
#include <algorithm>
#include <cstring>

ImageRgba::ImageRgba(const uint8_t* image, int width, int height) :
    m_ImageData{},
    m_Width{width},
    m_Height{height}
{
    int length = 4 * width * height;
    m_ImageData.insert(m_ImageData.begin(), image, image + length);
}

ImageRgba::ImageRgba(ImageRgba&& image) noexcept
{
    *this = std::move(image);
}

ImageRgba& ImageRgba::operator=(ImageRgba&& image) noexcept
{
    m_ImageData = std::move(image.m_ImageData);
    m_Width = std::exchange(image.m_Width, 0);
    m_Height = std::exchange(image.m_Height, 0);

    return *this;
}

const uint8_t* ImageRgba::GetRawImageData() const
{
    return m_ImageData.data();
}

int ImageRgba::GetWidth() const
{
    return m_Width;
}

int ImageRgba::GetHeight() const
{
    return m_Height;
}
