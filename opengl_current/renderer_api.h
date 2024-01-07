#pragma once

#include "vertex_array.h"
#include <cstdint>

/* Render primitives, which value is same as openGL equivalents*/
enum class RenderPrimitive
{
    kPoints = 0x0000, // GL_POINTS
    kLines = 0x0001, // GL_LINES
    kTriangles = 0x0004, // GL_TRIANGLES
    kTrianglesAdjancency = 0x000C // GL_TRIANGLES_ADJANCENCY
};

struct RgbaColor
{
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255) :
        red{red},
        green{green},
        blue{blue},
        alpha{alpha}
    {
    }
};


struct RgbColor
{
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) :
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

struct IndexedDrawData
{
    const VertexArray* vertex_array;
    std::int32_t num_indices{0};
    RenderPrimitive draw_primitive{RenderPrimitive::kTriangles};

    void Bind() const
    {
        vertex_array->Bind();
    }
};

class IRendererAPI
{
public:
    enum ApiType
    {
        kUnknown,
        kOpenGL
    };

public:
    virtual ~IRendererAPI() = default;

    virtual void Clear() = 0;
    virtual void SetClearColor(const RgbaColor& clear_color) = 0;
    virtual void DrawIndexed(const IndexedDrawData& draw_data) = 0;

    virtual void SetWireframe(bool wireframe_enabled) = 0;
    virtual bool IsWireframeEnabled() = 0;

    virtual void SetCullFace(bool cull_face) = 0;
    virtual bool DoesCullFaces() = 0;

    virtual void SetBlendingEnabled(bool blending_enabled) = 0;
    virtual void SetLineWidth(float lineWidth) = 0;

    virtual void ClearBufferBindings_Debug() = 0;
    virtual void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) = 0;

    inline static ApiType GetApi()
    {
        return renderer_api_type_;
    }

private:
    static ApiType renderer_api_type_;
};

