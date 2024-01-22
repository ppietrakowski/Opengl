#pragma once

#include "vertex_array.h"
#include "uniform_buffer.h"

#include <cstdint>

#include <span>
#include <glm/glm.hpp>

struct RgbaColor {
    uint8_t red{0};
    uint8_t green{0};
    uint8_t blue{0};
    uint8_t alpha{255};

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
    constexpr RgbaColor(const glm::vec4& color);
};

struct RgbColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

inline bool operator==(const RgbaColor& a, const RgbaColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}

inline bool operator!=(const RgbaColor& a, const RgbaColor& b) {
    return a.red != b.red || a.green != b.green || a.blue != b.blue || a.alpha != b.alpha;
}

enum class DepthFunction : uint8_t {
    Less = 0,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal
};

class RendererApi {
public:
    RendererApi() = default;
    ~RendererApi() = default;

    void Initialize();

    void Clear();
    void SetClearColor(const RgbaColor& clear_color);
    void DrawTriangles(const VertexArray& vertex_array, int num_indices);
    void DrawTrianglesArrays(const VertexArray& vertex_array, int num_vertices);

    void DrawTrianglesAdjancency(const VertexArray& vertex_array, int num_indices);
    void DrawLines(const VertexArray& vertex_array, int num_indices);
    void DrawPoints(const VertexArray& vertex_array, int num_indices);

    void SetWireframe(bool wireframe_enabled);
    bool IsWireframeEnabled() const;

    void SetCullFace(bool cull_faces);
    void UpdateCullFace(bool use_clockwise);
    bool DoesCullFaces() const;

    void SetBlendingEnabled(bool blending_enabled);
    void SetLineWidth(float lineWidth);

    void ClearBufferBindings_Debug();
    void SetViewport(int x, int y, int width, int height);

    void SetDepthFunc(DepthFunction depth_function);
    void DrawTrianglesInstanced(const VertexArray& vertex_array, int num_instances);

private:
    bool cull_enabled_ : 1 = false;
    bool wireframe_enabled_ : 1 = false;
    bool blending_enabled_ : 1 = false;
    RgbaColor clear_color_{0, 0, 0, 255};
};

FORCE_INLINE constexpr RgbaColor::RgbaColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
    red{red},
    green{green},
    blue{blue},
    alpha{alpha} {
}

#define CONVERT_FLOAT_TO_COMPONENT(FloatComponent) static_cast<uint8_t>((FloatComponent) * 255.0f)

FORCE_INLINE constexpr RgbaColor::RgbaColor(const glm::vec4& color) :
    red{CONVERT_FLOAT_TO_COMPONENT(color.r)},
    green{CONVERT_FLOAT_TO_COMPONENT(color.g)},
    blue{CONVERT_FLOAT_TO_COMPONENT(color.b)},
    alpha{CONVERT_FLOAT_TO_COMPONENT(color.a)} {
}

#undef CONVERT_FLOAT_TO_COMPONENT
