#include "render_command.h"
#include "opengl_render_api.h"

#include <chrono>

static RenderStats s_RenderStats;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
RendererAPI* RenderCommand::s_RendererApi = nullptr;

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

void RenderCommand::DrawIndexed(const IndexedDrawData& drawData)
{
    s_RendererApi->DrawIndexed(drawData);
    s_RenderStats.NumDrawcalls++;
}

void RenderCommand::BeginScene()
{
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

void RenderCommand::SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height)
{
    s_RendererApi->SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return s_RenderStats;
}

