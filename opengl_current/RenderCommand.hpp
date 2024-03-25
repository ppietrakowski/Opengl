#pragma once

#include "RendererApi.hpp"
#include "UniformBuffer.hpp"
#include <cstdint>

struct RenderStats
{
    int NumDrawcalls{0};
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

    static void DrawIndexed(const VertexArray& vertexArray, int numIndices);
    static void DrawArrays(const VertexArray& vertexArray, int numVertices);
    static void DrawLines(const VertexArray& vertexArray, int numIndices);
    static void DrawIndexedInstanced(const VertexArray& vertexArray, int numInstances);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clearColor);
    static void Clear();

    static void SetCullFace(bool bCullFaces);
    static bool DoesCullFaces();

    static void SetLineWidth(float line_width);
    static void SetViewport(int x, int y, int width, int height);

    static RenderStats GetRenderStats();

    static void SetDepthFunc(DepthFunction depthFunction);
    static void SetDepthEnabled(bool bDepthEnabled);
};
