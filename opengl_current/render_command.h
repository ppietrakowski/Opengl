#pragma once

#include "renderer_api.h"
#include "uniform_buffer.h"
#include <cstdint>

struct RenderStats
{
    size_t NumDrawcalls{0};
    int64_t DeltaFrameTime{0};
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

    static void DrawIndexed(const VertexArray& vertexArray, int32_t numIndices);
    static void DrawArrays(const VertexArray& vertexArray, int32_t numVertices);
    static void DrawLines(const VertexArray& vertexArray, int32_t numIndices);
    static void DrawIndexedInstanced(const VertexArray& vertexArray, int32_t numInstances);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clearColor);
    static void Clear();

    static void SetCullFace(bool bCullFaces);
    static bool DoesCullFaces();

    static void SetLineWidth(float line_width);
    static void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height);

    static RenderStats GetRenderStats();

    static void SetDepthFunc(DepthFunction depthFunction);
    static void SetDepthEnabled(bool bDepthEnabled);
};

