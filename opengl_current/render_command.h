#pragma once

#include "renderer_api.h"
#include <cstdint>

struct RenderStats
{
    int32_t NumDrawCalls{0};
    int32_t NumTriangles{0};
    int64_t DeltaFrameNanoseconds{0};
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

    static void DrawIndexed(const IndexedDrawData& drawData);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clearColor);
    static void Clear();

    static void SetWireframe(bool bWireframeEnabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool bCullFace);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool bBlendingEnabled);

    static void SetLineWidth(float lineWidth);
    static void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height);

    static RenderStats GetRenderStats();

private:
    static IRendererAPI* RendererApi;
};

