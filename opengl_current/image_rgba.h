#pragma once

#include <cstdint>

inline void DefaultImageDataDeleter(uint8_t* data)
{
    delete[] data;
}

class ImageRgba
{
public:
    ImageRgba(uint8_t* image, int32_t width, int32_t height, void (*deleter)(uint8_t*) = &DefaultImageDataDeleter);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;
    ~ImageRgba();

    const uint8_t* GetRawImageData() const;

    int32_t GetWidth() const;
    int32_t GetHeight() const;

private:
    uint8_t* ImageData;
    int32_t Width;
    int32_t Height;
    void (*Deleter)(uint8_t*);
};
