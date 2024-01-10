#pragma once

#include <cstdint>

inline void DefaultImageDataDeleter(uint8_t* data)
{
    delete[] data;
}

class ImageRgba
{
public:
    ImageRgba(std::uint8_t* image, std::int32_t width, std::int32_t height, void (*deleter)(std::uint8_t*) = &DefaultImageDataDeleter);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;
    ~ImageRgba();

    const std::uint8_t* GetRawImageData() const;

    std::int32_t GetWidth() const;
    std::int32_t GetHeight() const;

private:
    std::uint8_t* m_ImageData;
    std::int32_t m_Width;
    std::int32_t m_Height;
    void (*m_Deleter)(std::uint8_t*);
};
