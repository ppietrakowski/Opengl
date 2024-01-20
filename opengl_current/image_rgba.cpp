#include "image_rgba.h"

#include <algorithm>
#include <cstring>

ImageRgba::ImageRgba(uint8_t* image, int width, int height, void(*deleter)(uint8_t*)) :
    image_data_{image},
    width_{width},
    height_{height},
    deleter_{deleter} {
}

ImageRgba::ImageRgba(ImageRgba&& image) noexcept {
    *this = std::move(image);
}

ImageRgba& ImageRgba::operator=(ImageRgba&& image) noexcept {
    image_data_ = image.image_data_;
    width_ = image.width_;
    height_ = image.height_;
    deleter_ = image.deleter_;
    memset(&image, 0, sizeof(image));

    return *this;
}

ImageRgba::~ImageRgba() {
    deleter_(image_data_);
}

const uint8_t* ImageRgba::GetRawImageData() const {
    return image_data_;
}

int ImageRgba::GetWidth() const {
    return width_;
}

int ImageRgba::GetHeight() const {
    return height_;
}
