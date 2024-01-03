#include "render_command.h"
#include "opengl_render_api.h"

#include <chrono>

static RenderStats s_RenderStats;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
IRendererAPI* RenderCommand::RendererApi = nullptr;

void RenderCommand::Initialize()
{
    RendererApi = new OpenGlRenderApi();
}

void RenderCommand::Quit()
{
    delete RendererApi;
    RendererApi = nullptr;
}

void RenderCommand::ClearBufferBindings_Debug()
{
    RendererApi->ClearBufferBindings_Debug();
}

void RenderCommand::DrawIndexed(const IndexedDrawData& drawData)
{
    RendererApi->DrawIndexed(drawData);
    s_RenderStats.NumDrawCalls++;
    s_RenderStats.NumTriangles += (drawData.NumIndices / 3);
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
    RendererApi->SetClearColor(clearColor);
}

void RenderCommand::Clear()
{
    RendererApi->Clear();
}

void RenderCommand::SetWireframe(bool bWireframeEnabled)
{
    RendererApi->SetWireframe(bWireframeEnabled);
}

bool RenderCommand::IsWireframeEnabled()
{
    return RendererApi->IsWireframeEnabled();
}

void RenderCommand::SetCullFace(bool bCullFace)
{
    RendererApi->SetCullFace(bCullFace);
}

bool RenderCommand::DoesCullFaces()
{
    return RendererApi->DoesCullFaces();
}

void RenderCommand::SetBlendingEnabled(bool bBlendingEnabled)
{
    RendererApi->SetBlendingEnabled(bBlendingEnabled);
}

void RenderCommand::SetLineWidth(float lineWidth)
{
    RendererApi->SetLineWidth(lineWidth);
}

void RenderCommand::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    RendererApi->SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return s_RenderStats;
}

