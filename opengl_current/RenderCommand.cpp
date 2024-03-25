#include "RenderCommand.hpp"
#include "ErrorMacros.hpp"

#include <chrono>

static RenderStats s_RenderStats;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
static bool s_bRenderCommandInitialized = false;
static RendererApi s_RendererApi{};

void RenderCommand::Initialize()
{
    s_RendererApi.Initialize();
    s_bRenderCommandInitialized = true;
}

void RenderCommand::Quit()
{
    s_bRenderCommandInitialized = false;
}

void RenderCommand::ClearBufferBindings_Debug()
{
    ASSERT(s_bRenderCommandInitialized);
    s_RendererApi.ClearBufferBindings_Debug();
}

void RenderCommand::DrawIndexed(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(s_bRenderCommandInitialized);
    s_RendererApi.DrawIndexed(vertexArray, numIndices);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::DrawArrays(const VertexArray& vertexArray, int numVertices)
{
    s_RendererApi.DrawArrays(vertexArray, numVertices);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::DrawLines(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawLines(vertexArray, numIndices);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::DrawIndexedInstanced(const VertexArray& vertexArray, int numInstances)
{
    s_RendererApi.DrawIndexedInstanced(vertexArray, numInstances);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::BeginScene()
{
    ASSERT(s_bRenderCommandInitialized);

    s_RenderStats.NumDrawcalls = 0;
}

void RenderCommand::EndScene()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    s_RenderStats.DeltaFrameTime = (now - s_StartTimestamp).count();
    s_StartTimestamp = now;
}

void RenderCommand::SetClearColor(const RgbaColor& clearColor)
{
    s_RendererApi.SetClearColor(clearColor);
}

void RenderCommand::Clear()
{
    s_RendererApi.Clear();
}

void RenderCommand::SetCullFace(bool bCullFaces)
{
    s_RendererApi.SetCullFace(bCullFaces);
}

bool RenderCommand::DoesCullFaces()
{
    return s_RendererApi.DoesCullFaces();
}

void RenderCommand::SetLineWidth(float lineWidth)
{
    s_RendererApi.SetLineWidth(lineWidth);
}

void RenderCommand::SetViewport(int x, int y, int width, int height)
{
    s_RendererApi.SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return s_RenderStats;
}

void RenderCommand::SetDepthFunc(DepthFunction depthFunction)
{
    s_RendererApi.SetDepthFunc(depthFunction);
}

void RenderCommand::SetDepthEnabled(bool bDepthEnabled)
{
    s_RendererApi.SetDepthEnabled(bDepthEnabled);
}