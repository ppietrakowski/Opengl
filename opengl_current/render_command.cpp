#include "render_command.h"
#include "error_macros.h"

#include <chrono>

static RenderStats s_RenderStats;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
RendererApi RenderCommand::s_RendererApi{};
static bool s_bRenderCommandInitialized = false;

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

void RenderCommand::DrawTriangles(const VertexArray& vertexArray, std::int32_t numIndices)
{
    ASSERT(s_bRenderCommandInitialized);
    s_RendererApi.DrawTriangles(vertexArray, numIndices);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::DrawTrianglesAdjancency(const VertexArray& vertexArray, std::int32_t numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawTrianglesAdjancency(vertexArray, numIndices);
    s_RenderStats.NumDrawcalls++;
}
void RenderCommand::DrawLines(const VertexArray& vertexArray, std::int32_t numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawLines(vertexArray, numIndices);
    s_RenderStats.NumDrawcalls++;
}
void RenderCommand::DrawPoints(const VertexArray& vertexArray, std::int32_t numIndices)
{
    ASSERT(s_bRenderCommandInitialized);

    s_RendererApi.DrawPoints(vertexArray, numIndices);
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
    s_RenderStats.DeltaFrameNanoseconds = (now - s_StartTimestamp).count();
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

void RenderCommand::SetCullFace(bool bCullFace)
{
    s_RendererApi.SetCullFace(bCullFace);
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

void RenderCommand::SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height)
{
    s_RendererApi.SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return s_RenderStats;
}

void RenderCommand::NotifyIndexBufferCreated(std::int32_t bufferSize)
{
    s_RenderStats.NumIndexBufferMemoryAllocated += bufferSize;
}

void RenderCommand::NotifyIndexBufferDestroyed(std::int32_t bufferSize)
{
    s_RenderStats.NumIndexBufferMemoryAllocated -= bufferSize;
}

void RenderCommand::NotifyVertexBufferCreated(std::int32_t bufferSize)
{
    s_RenderStats.NumVertexBufferMemoryAllocated += bufferSize;
}

void RenderCommand::NotifyVertexBufferDestroyed(std::int32_t bufferSize)
{
    s_RenderStats.NumVertexBufferMemoryAllocated -= bufferSize;
}