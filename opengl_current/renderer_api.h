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
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
    uint8_t Alpha;

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) :
        Red{red},
        Green{green},
        Blue{blue},
        Alpha{alpha}
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
    const IVertexArray* VertexArray;
    int32_t NumIndices{0};
    RenderPrimitive DrawPrimitive{RenderPrimitive::kTriangles};

    void Bind() const
    {
        VertexArray->Bind();
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
    virtual void SetClearColor(const RgbaColor& clearColor) = 0;
    virtual void DrawIndexed(const IndexedDrawData& drawData) = 0;

    virtual void SetWireframe(bool bWireframeEnabled) = 0;
    virtual bool IsWireframeEnabled() = 0;

    virtual void SetCullFace(bool bCullFaces) = 0;
    virtual bool DoesCullFaces() = 0;

    virtual void SetBlendingEnabled(bool bBlendingEnabled) = 0;
    virtual void SetLineWidth(float lineWidth) = 0;

    virtual void ClearBufferBindings_Debug() = 0;
    virtual void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height) = 0;

    inline static ApiType GetApi()
    {
        return RenderApiType;
    }

private:
    static ApiType RenderApiType;
};

