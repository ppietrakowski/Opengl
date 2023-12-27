#pragma once

#include "vertex_array.h"
#include <cstdint>

/* Render primitives, which value is same as openGL equivalents*/
enum class RenderPrimitive {
    kPoints = 0x0000, // GL_POINTS
    kLines = 0x0001, // GL_LINES
    kTriangles = 0x0004, // GL_TRIANGLES
    kTrianglesAdjancency = 0x000C // GL_TRIANGLES_ADJANCENCY
};

struct RgbaColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) :
        red{ red },
        green{ green },
        blue{ blue },
        alpha{ alpha } {}
};

inline bool operator==(const RgbaColor& a, const RgbaColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}

inline bool operator!=(const RgbaColor& a, const RgbaColor& b) {
    return a.red != b.red || a.green != b.green || a.blue != b.blue || a.alpha != b.alpha;
}

class RendererAPI {
public:
    enum ApiType {
        kUnknown,
        kOpenGL
    };

public:
    virtual ~RendererAPI() = default;

    virtual void Clear() = 0;
    virtual void SetClearColor(const RgbaColor& clear_color) = 0;
    virtual void DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive) = 0;

    virtual void SetWireframe(bool wireframe_enabled) = 0;
    virtual bool IsWireframeEnabled() = 0;

    virtual void SetCullFace(bool cull_face) = 0;
    virtual bool DoesCullFaces() = 0;

    virtual void SetBlendingEnabled(bool blending_enabled) = 0;
    virtual void SetLineWidth(float line_width) = 0;

    virtual void ClearBufferBindings_Debug() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

    inline static ApiType GetApi() {
        return render_api_type_;
    }

private:
    static ApiType render_api_type_;
};

