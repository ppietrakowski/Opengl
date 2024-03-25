#pragma once

struct CameraProjection
{
    float Fov{45.0f};
    float AspectRatio{0.0f};
    float Width{0};
    float Height{0};
    float ZNear{0.1f};
    float ZFar{1000.0f};

    CameraProjection(int width, int height, float fov = 45.0f, float zNear = 0.001f, float zFar = 1000.0f);

    CameraProjection() = default;
    CameraProjection(const CameraProjection&) = default;
    CameraProjection& operator=(const CameraProjection&) = default;
};


FORCE_INLINE CameraProjection::CameraProjection(int width, int height, float fov, float zNear, float zFar) :
    Width(static_cast<float>(width)),
    Height(static_cast<float>(height)),
    Fov(fov),
    ZNear(zNear),
    ZFar(zFar)
{
    AspectRatio = Width / Height;
}
