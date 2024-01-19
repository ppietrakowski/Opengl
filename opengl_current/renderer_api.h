#pragma once

#include "vertex_array.h"
#include "uniform_buffer.h"

#include <cstdint>

#include <span>
#include <glm/glm.hpp>

#define CONVERT_FLOAT_TO_COMPONENT(FloatComponent) static_cast<uint8_t>((FloatComponent) * 255.0f)

struct RgbaColor
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) :
        red{red},
        green{green},
        blue{blue},
        alpha{alpha}
    {
    }

    constexpr RgbaColor(const glm::vec4& color):
        red{CONVERT_FLOAT_TO_COMPONENT(color.r)},
        green{CONVERT_FLOAT_TO_COMPONENT(color.g)},
        blue{CONVERT_FLOAT_TO_COMPONENT(color.b)},
        alpha{CONVERT_FLOAT_TO_COMPONENT(color.a)}
    {
    }
};

#undef CONVERT_FLOAT_TO_COMPONENT


struct RgbColor
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(uint8_t red, uint8_t green, uint8_t blue) :
        red{red},
        green{green},
        blue{blue}
    {
    }
};

inline bool operator==(const RgbaColor& a, const RgbaColor& b)
{
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}

inline bool operator!=(const RgbaColor& a, const RgbaColor& b)
{
    return a.red != b.red || a.green != b.green || a.blue != b.blue || a.alpha != b.alpha;
}

class RendererApi
{
public:
    RendererApi() = default;
    ~RendererApi() = default;

    void Initialize();

    void Clear();
    void SetClearColor(const RgbaColor& clear_color);
    void DrawTriangles(const VertexArray& vertex_array, int num_indices);
    void DrawTrianglesAdjancency(const VertexArray& vertex_array, int num_indices);
    void DrawLines(const VertexArray& vertex_array, int num_indices) ;
    void DrawPoints(const VertexArray& vertex_array, int num_indices);

    void SetWireframe(bool wireframe_enabled);
    bool IsWireframeEnabled() const;

    void SetCullFace(bool cull_faces);
    bool DoesCullFaces() const;

    void SetBlendingEnabled(bool blending_enabled);
    void SetLineWidth(float lineWidth);

    void ClearBufferBindings_Debug();
    void SetViewport(int x, int y, int width, int height);

    void DrawTrianglesInstanced(const VertexArray& vertex_array, int num_instances);

private:
    bool cull_enabled_      : 1 = false;
    bool wireframe_enabled_ : 1 = false;
    bool blending_enabled_  : 1 = false;
    RgbaColor clear_color_{0, 0, 0, 255};
};

