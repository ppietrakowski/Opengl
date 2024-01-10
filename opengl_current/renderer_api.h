#pragma once

#include "vertex_array.h"
#include <cstdint>

/* Render primitives, which value is same as openGL equivalents*/
enum class RenderPrimitive
{
    Points = 0x0000, // GL_POINTS
    Lines = 0x0001, // GL_LINES
    Triangles = 0x0004, // GL_TRIANGLES
    TrianglesAdjancency = 0x000C // GL_TRIANGLES_ADJANCENCY
};

struct RgbaColor
{
    std::uint8_t Red;
    std::uint8_t Green;
    std::uint8_t Blue;
    std::uint8_t Alpha;

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255) :
        Red{red},
        Green{green},
        Blue{blue},
        Alpha{alpha}
    {
    }
};


struct RgbColor
{
    std::uint8_t Red;
    std::uint8_t Green;
    std::uint8_t Blue;

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) :
        Red{red},
        Green{green},
        Blue{blue}
    {
    }
};

inline bool operator==(const RgbaColor& a, const RgbaColor& b)
{
    return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue && a.Alpha == b.Alpha;
}

inline bool operator!=(const RgbaColor& a, const RgbaColor& b)
{
    return a.Red != b.Red || a.Green != b.Green || a.Blue != b.Blue || a.Alpha != b.Alpha;
}

struct IndexedDrawData
{
    const VertexArray* TargetVertexArray;
    std::int32_t NumIndices{0};
    RenderPrimitive DrawPrimitive{RenderPrimitive::Triangles};

    void Bind() const
    {
        TargetVertexArray->Bind();
    }
};

class RendererAPI
{
public:
    enum ApiType
    {
        Unknown,
        OpenGL
    };

public:
    virtual ~RendererAPI() = default;

    virtual void Clear() = 0;
    virtual void SetClearColor(const RgbaColor& clearColor) = 0;
    virtual void DrawTriangles(const VertexArray& vertexArray, std::int32_t numIndices) = 0;
    virtual void DrawTrianglesAdjancency(const VertexArray& vertexArray, std::int32_t numIndices) = 0;
    virtual void DrawLines(const VertexArray& vertexArray, std::int32_t numIndices) = 0;
    virtual void DrawPoints(const VertexArray& vertexArray, std::int32_t numIndices) = 0;

    virtual void SetWireframe(bool bWireframeEnabled) = 0;
    virtual bool IsWireframeEnabled() = 0;

    virtual void SetCullFace(bool bCullFace) = 0;
    virtual bool DoesCullFaces() = 0;

    virtual void SetBlendingEnabled(bool bBlendingEnabled) = 0;
    virtual void SetLineWidth(float lineWidth) = 0;

    virtual void ClearBufferBindings_Debug() = 0;
    virtual void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) = 0;

    inline static ApiType GetApi()
    {
        return s_RendererApiType;
    }

private:
    static ApiType s_RendererApiType;
};

