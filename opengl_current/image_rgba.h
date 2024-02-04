#pragma once

#include <cstdint>
#include <memory>

inline void DefaultImageDataDeleter(uint8_t* data)
{
    delete[] data;
}

struct DefaultImageDeleter
{
    using DeleterFunc = void (*)(uint8_t* data);

    DeleterFunc Func{&DefaultImageDataDeleter};

    DefaultImageDeleter() = default;
    DefaultImageDeleter(DeleterFunc deleteFunc) :
        Func{deleteFunc}
    {
    }

    void operator()(uint8_t* data)
    {
        Func(data);
    }
};


class ImageRgba
{
public:
    ImageRgba(uint8_t* image, int32_t width, int32_t height, void (*deleter)(uint8_t*) = &DefaultImageDataDeleter);

    ImageRgba(ImageRgba&& image) noexcept;
    ImageRgba& operator=(ImageRgba&& image) noexcept;

    const uint8_t* GetRawImageData() const;

    int32_t GetWidth() const;
    int32_t GetHeight() const;

private:
    std::unique_ptr<uint8_t, DefaultImageDeleter> m_ImageData;
    int32_t m_Width;
    int32_t m_Height;
};
