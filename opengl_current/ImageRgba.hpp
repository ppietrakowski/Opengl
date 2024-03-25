#pragma once

#include <cstdint>
#include <memory>
#include <vector>

class ImageRgba
{
public:
    ImageRgba(const uint8_t* image, int width, int height);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;

    const uint8_t* GetRawImageData() const;

    int GetWidth() const;
    int GetHeight() const;

private:
    std::vector<uint8_t> m_ImageData;
    int m_Width;
    int m_Height;
};
