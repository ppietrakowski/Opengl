#pragma once

#include "vertex_array.h"
#include <cstdint>

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

class RendererApi
{
public:
    RendererApi() = default;
    ~RendererApi() = default;

    void Initialize();

    void Clear();
    void SetClearColor(const RgbaColor& clearColor);
    void DrawTriangles(const VertexArray& vertexArray, std::int32_t numIndices);
    void DrawTrianglesAdjancency(const VertexArray& vertexArray, std::int32_t numIndices);
    void DrawLines(const VertexArray& vertexArray, std::int32_t numIndices) ;
    void DrawPoints(const VertexArray& vertexArray, std::int32_t numIndices);

    void SetWireframe(bool bWireframeEnabled);
    bool IsWireframeEnabled();

    void SetCullFace(bool bCullFace);
    bool DoesCullFaces();

    void SetBlendingEnabled(bool bBlendingEnabled);
    void SetLineWidth(float lineWidth);

    void ClearBufferBindings_Debug();
    void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height);

private:
    bool m_bCullEnabled      : 1 = false;
    bool m_bWireframeEnabled : 1 = false;
    bool m_bBlendingEnabled  : 1 = false;
    RgbaColor m_ClearColor{0, 0, 0, 255};
};

