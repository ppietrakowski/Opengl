#include "render_command.h"
#include "opengl_render_api.h"

#include <chrono>

static RenderStats s_RenderStats;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
IRendererAPI* RenderCommand::s_RendererApi = nullptr;

void RenderCommand::Initialize()
{
    s_RendererApi = new OpenGlRenderApi();
}

void RenderCommand::Quit()
{
    delete s_RendererApi;
    s_RendererApi = nullptr;
}

void RenderCommand::ClearBufferBindings_Debug()
{
    s_RendererApi->ClearBufferBindings_Debug();
}

void RenderCommand::DrawIndexed(const IVertexArray& vertexArray, int32_t numIndices, RenderPrimitive renderPrimitive)
{
    s_RendererApi->DrawIndexed(vertexArray, numIndices, renderPrimitive);
    s_RenderStats.NumDrawCalls++;
    s_RenderStats.NumTriangles += (numIndices / 3);
}

void RenderCommand::BeginScene()
{
    s_RenderStats.NumDrawCalls = 0;
    s_RenderStats.NumTriangles = 0;
}

void RenderCommand::EndScene()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    s_RenderStats.DeltaFrameNanoseconds = (now - s_StartTimestamp).count();
    s_StartTimestamp = now;
}

void RenderCommand::SetClearColor(const RgbaColor& clearColor)
{
    s_RendererApi->SetClearColor(clearColor);
}

void RenderCommand::Clear()
{
    s_RendererApi->Clear();
}

void RenderCommand::SetWireframe(bool bWireframeEnabled)
{
    s_RendererApi->SetWireframe(bWireframeEnabled);
}

bool RenderCommand::IsWireframeEnabled()
{
    return s_RendererApi->IsWireframeEnabled();
}

void RenderCommand::SetCullFace(bool bCullFace)
{
    s_RendererApi->SetCullFace(bCullFace);
}

bool RenderCommand::DoesCullFaces()
{
    return s_RendererApi->DoesCullFaces();
}

void RenderCommand::SetBlendingEnabled(bool bBlendingEnabled)
{
    s_RendererApi->SetBlendingEnabled(bBlendingEnabled);
}

void RenderCommand::SetLineWidth(float lineWidth)
{
    s_RendererApi->SetLineWidth(lineWidth);
}

void RenderCommand::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    s_RendererApi->SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return s_RenderStats;
}

