#include "render_command.h"
#include "error_macros.h"

#include <chrono>

static RenderStats s_RenderStats;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
RendererApi RenderCommand::s_RendererApi{};
static bool s_bRenderCommandInitialized = false;

static FORCE_INLINE int GetNumIndices(int numIndices, const VertexArray& vertexArray)
{
    return numIndices == 0 ? vertexArray.GetNumIndices() : numIndices;
}

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
    s_RendererApi.DrawTriangles(vertexArray, GetNumIndices(numIndices, vertexArray));
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::DrawTrianglesArrays(const VertexArray& vertexArray, int numVertices)
{
    s_RendererApi.DrawTrianglesArrays(vertexArray, numVertices);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::DrawTrianglesAdjancency(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawTrianglesAdjancency(vertexArray, GetNumIndices(numIndices, vertexArray));
    s_RenderStats.NumDrawcalls++;
}
void RenderCommand::DrawLines(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawLines(vertexArray, GetNumIndices(numIndices, vertexArray));
    s_RenderStats.NumDrawcalls++;
}
void RenderCommand::DrawPoints(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawPoints(vertexArray, GetNumIndices(numIndices, vertexArray));
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

void RenderCommand::SetWireframe(bool bWireframeEnabled)
{
    s_RendererApi.SetWireframe(bWireframeEnabled);
}

bool RenderCommand::IsWireframeEnabled()
{
    return s_RendererApi.IsWireframeEnabled();
}

void RenderCommand::SetCullFace(bool bCullFaces)
{
    s_RendererApi.SetCullFace(bCullFaces);
}

void RenderCommand::UpdateCullFace(bool bUseClockwise)
{
    s_RendererApi.UpdateCullFace(bUseClockwise);
}

bool RenderCommand::DoesCullFaces()
{
    return s_RendererApi.DoesCullFaces();
}

void RenderCommand::SetBlendingEnabled(bool bBlendingEnabled)
{
    s_RendererApi.SetBlendingEnabled(bBlendingEnabled);
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

void RenderCommand::NotifyIndexBufferCreated(int bufferSize)
{
    s_RenderStats.IndexBufferMemoryAllocation += bufferSize;
}

void RenderCommand::NotifyIndexBufferDestroyed(int bufferSize)
{
    s_RenderStats.IndexBufferMemoryAllocation -= bufferSize;
}

void RenderCommand::NotifyVertexBufferCreated(int bufferSize)
{
    s_RenderStats.VertexBufferMemoryAllocation += bufferSize;
}

void RenderCommand::NotifyVertexBufferDestroyed(int bufferSize)
{
    s_RenderStats.VertexBufferMemoryAllocation -= bufferSize;
}

void RenderCommand::DrawIndexedInstanced(const VertexArray& vertexArray, int numInstances)
{
    s_RendererApi.DrawTrianglesInstanced(vertexArray, numInstances);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::SetDepthFunc(DepthFunction depthFunction)
{
    s_RendererApi.SetDepthFunc(depthFunction);
}


void RenderCommand::SetDepthEnabled(bool bEnabled)
{
    s_RendererApi.SetDepthEnabled(bEnabled);
}