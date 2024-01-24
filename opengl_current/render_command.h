#pragma once

#include "renderer_api.h"
#include "uniform_buffer.h"
#include <cstdint>

struct RenderStats
{
    size_t NumDrawcalls{0};
    int64_t DeltaFrameTime{0};
    size_t IndexBufferMemoryAllocation{0};
    size_t VertexBufferMemoryAllocation{0};
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

    static void DrawTriangles(const VertexArray& vertexArray, int numIndices);
    static void DrawTrianglesArrays(const VertexArray& vertexArray, int numVertices);
    static void DrawTrianglesAdjancency(const VertexArray& vertexArray, int numIndices);
    static void DrawLines(const VertexArray& vertexArray, int numIndices);
    static void DrawPoints(const VertexArray& vertexArray, int numIndices);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clearColor);
    static void Clear();

    static void SetWireframe(bool bWireframeEnabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool bCullFaces);
    static void UpdateCullFace(bool bUseClockwise);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool bBlendingEnabled);

    static void SetLineWidth(float line_width);
    static void SetViewport(int x, int y, int width, int height);

    static RenderStats GetRenderStats();

    static void NotifyIndexBufferCreated(int bufferSize);
    static void NotifyIndexBufferDestroyed(int bufferSize);

    static void NotifyVertexBufferCreated(int bufferSize);
    static void NotifyVertexBufferDestroyed(int bufferSize);

    static void DrawTrianglesInstanced(const VertexArray& vertexArray, int numInstances);
    static void SetDepthFunc(DepthFunction depthFunction);
    static void SetDepthEnabled(bool bEnabled);

private:
    static RendererApi s_RendererApi;
};

