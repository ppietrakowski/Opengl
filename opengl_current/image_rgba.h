#pragma once

#include <cstdint>

inline void DefaultImageDataDeleter(uint8_t* data)
{
    delete[] data;
}

class ImageRgba
{
public:
    ImageRgba(uint8_t* image, uint32_t width, uint32_t height, void (*deleter)(uint8_t*) = &DefaultImageDataDeleter);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;
    ~ImageRgba();

    const uint8_t* GetRawImageData() const;

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

private:
    uint8_t* m_ImageData;
    uint32_t m_Width;
    uint32_t m_Height;
    void (*m_Deleter)(uint8_t*);
};
