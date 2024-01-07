#pragma once

#include <cstdint>

inline void DefaultImageDataDeleter(uint8_t* data) {
    delete[] data;
}

class ImageRgba {
public:
    ImageRgba(std::uint8_t* image, std::int32_t width, std::int32_t height, void (*deleter)(std::uint8_t*) = &DefaultImageDataDeleter);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;
    ~ImageRgba();

    const std::uint8_t* GetRawImageData() const;

    std::int32_t GetWidth() const;
    std::int32_t GetHeight() const;

private:
    std::uint8_t* image_data_;
    std::int32_t width_;
    std::int32_t height_;
    void (*deleter_)(std::uint8_t*);
};
