#pragma once

#include "VertexArray.hpp"
#include "UniformBuffer.hpp"

#include <cstdint>

#include <span>
#include <glm/glm.hpp>

struct RgbaColor
{
    uint8_t Red{0};
    uint8_t Green{0};
    uint8_t Blue{0};
    uint8_t Alpha{255};

    RgbaColor() = default;
    RgbaColor(const RgbaColor&) = default;
    RgbaColor& operator=(const RgbaColor&) = default;

    constexpr RgbaColor(uint8_t red, uint8_t Green, uint8_t Blue, uint8_t Alpha = 255);
    constexpr RgbaColor(const glm::vec4& Color);
};

struct RgbColor
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

inline bool operator==(const RgbaColor& a, const RgbaColor& b)
{
    return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue && a.Alpha == b.Alpha;
}

inline bool operator!=(const RgbaColor& a, const RgbaColor& b)
{
    return a.Red != b.Red || a.Green != b.Green || a.Blue != b.Blue || a.Alpha != b.Alpha;
}

enum class DepthFunction : uint8_t
{
    Less = 0,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal
};

class RendererApi
{
public:
    RendererApi() = default;
    ~RendererApi() = default;

    void Initialize();

    void Clear();
    void SetClearColor(const RgbaColor& clearColor);

    void DrawIndexed(const VertexArray& vertexArray, int numIndices);
    void DrawArrays(const VertexArray& vertexArray, int numVertices);
    void DrawLines(const VertexArray& vertexArray, int numIndices);
    void DrawIndexedInstanced(const VertexArray& vertexArray, int numInstances);

    void SetCullFace(bool bCullFaces);
    bool DoesCullFaces() const;

    void SetLineWidth(float lineWidth);

    void ClearBufferBindings_Debug();
    void SetViewport(int x, int y, int width, int height);

    void SetDepthFunc(DepthFunction depthFunction);
    void SetDepthEnabled(bool bDepthEnabled);

private:
    bool m_bCullFaces : 1 = true;
    RgbaColor m_ClearColor{0, 0, 0, 255};
};

FORCE_INLINE constexpr RgbaColor::RgbaColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
    Red{red},
    Green{green},
    Blue{blue},
    Alpha{alpha}
{
}

#define CONVERT_FLOAT_TO_COMPONENT(FloatComponent) static_cast<uint8_t>((FloatComponent) * 255.0f)

FORCE_INLINE constexpr RgbaColor::RgbaColor(const glm::vec4& Color) :
    Red{CONVERT_FLOAT_TO_COMPONENT(Color.r)},
    Green{CONVERT_FLOAT_TO_COMPONENT(Color.g)},
    Blue{CONVERT_FLOAT_TO_COMPONENT(Color.b)},
    Alpha{CONVERT_FLOAT_TO_COMPONENT(Color.a)}
{
}

#undef CONVERT_FLOAT_TO_COMPONENT
