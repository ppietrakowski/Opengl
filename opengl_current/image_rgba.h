#pragma once

#include <cstdint>

inline void DefaultImageDataDeleter(uint8_t* data) {
    delete[] data;
}

class ImageRgba {
public:
    ImageRgba(uint8_t* image, int width, int height, void (*deleter)(uint8_t*) = &DefaultImageDataDeleter);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;
    ~ImageRgba();

    const uint8_t* GetRawImageData() const;

    int GetWidth() const;
    int GetHeight() const;

private:
    uint8_t* image_data_;
    int width_;
    int height_;
    void (*deleter_)(uint8_t*);
};
