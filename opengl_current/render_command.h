#pragma once

#include "renderer_api.h"
#include <cstdint>

struct RenderStats
{
    std::int32_t NumDrawcalls{0};
    std::int64_t DeltaFrameNanoseconds{0};
};

class RenderCommand
{
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void Initialize();
    static void Quit();

    static void ClearBufferBindings_Debug();

    static void DrawTriangles(const VertexArray& vertexArray, std::int32_t numIndices);
    static void DrawTrianglesAdjancency(const VertexArray& vertexArray, std::int32_t numIndices);
    static void DrawLines(const VertexArray& vertexArray, std::int32_t numIndices);
    static void DrawPoints(const VertexArray& vertexArray, std::int32_t numIndices);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clearColor);
    static void Clear();

    static void SetWireframe(bool bWireframeEnabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool bCullFaces);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool bBlendingEnabled);

    static void SetLineWidth(float lineWidth);
    static void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height);

    static RenderStats GetRenderStats();

private:
    static RendererAPI* s_RendererApi;
};

