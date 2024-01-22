#pragma once

struct CameraProjection {
    float fov{45.0f};
    float aspect_ratio{0.0f};
    float width{0};
    float height{0};
    float z_near{0.1f};
    float z_far{1000.0f};

    CameraProjection(uint32_t width, uint32_t height, float fov = 45.0f, float z_near = 0.1f, float z_far = 1000.0f);

    CameraProjection() = default;
    CameraProjection(const CameraProjection&) = default;
    CameraProjection& operator=(const CameraProjection&) = default;
};


FORCE_INLINE CameraProjection::CameraProjection(uint32_t width, uint32_t height, float fov, float z_near, float z_far) :
    width(static_cast<float>(width)),
    height(static_cast<float>(height)),
    fov(fov),
    z_near(z_near),
    z_far(z_far) {
    aspect_ratio = this->width / this->height;
}
