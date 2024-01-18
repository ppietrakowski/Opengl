#include "render_command.h"
#include "error_macros.h"

#include <chrono>

static RenderStats render_stats_;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
RendererApi RenderCommand::renderer_api_{};
static bool s_bRenderCommandInitialized = false;

void RenderCommand::Initialize()
{
    renderer_api_.Initialize();
    s_bRenderCommandInitialized = true;
}

void RenderCommand::Quit()
{
    s_bRenderCommandInitialized = false;
}

void RenderCommand::ClearBufferBindings_Debug()
{
    ASSERT(s_bRenderCommandInitialized);
    renderer_api_.ClearBufferBindings_Debug();
}

void RenderCommand::DrawTriangles(const VertexArray& vertex_array, std::int32_t num_indices)
{
    ASSERT(s_bRenderCommandInitialized);
    renderer_api_.DrawTriangles(vertex_array, num_indices);
    render_stats_.num_drawcalls++;
}

void RenderCommand::DrawTrianglesAdjancency(const VertexArray& vertex_array, std::int32_t num_indices)
{
    ASSERT(s_bRenderCommandInitialized);

    renderer_api_.DrawTrianglesAdjancency(vertex_array, num_indices);
    render_stats_.num_drawcalls++;
}
void RenderCommand::DrawLines(const VertexArray& vertex_array, std::int32_t num_indices)
{
    ASSERT(s_bRenderCommandInitialized);

    renderer_api_.DrawLines(vertex_array, num_indices);
    render_stats_.num_drawcalls++;
}
void RenderCommand::DrawPoints(const VertexArray& vertex_array, std::int32_t num_indices)
{
    ASSERT(s_bRenderCommandInitialized);

    renderer_api_.DrawPoints(vertex_array, num_indices);
    render_stats_.num_drawcalls++;
}

void RenderCommand::BeginScene()
{
    ASSERT(s_bRenderCommandInitialized);

    render_stats_.num_drawcalls = 0;
}

void RenderCommand::EndScene()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    render_stats_.delta_frame_time = (now - s_StartTimestamp).count();
    s_StartTimestamp = now;
}

void RenderCommand::SetClearColor(const RgbaColor& clear_color)
{
    renderer_api_.SetClearColor(clear_color);
}

void RenderCommand::Clear()
{
    renderer_api_.Clear();
}

void RenderCommand::SetWireframe(bool wireframe_enabled)
{
    renderer_api_.SetWireframe(wireframe_enabled);
}

bool RenderCommand::IsWireframeEnabled()
{
    return renderer_api_.IsWireframeEnabled();
}

void RenderCommand::SetCullFace(bool cull_faces)
{
    renderer_api_.SetCullFace(cull_faces);
}

bool RenderCommand::DoesCullFaces()
{
    return renderer_api_.DoesCullFaces();
}

void RenderCommand::SetBlendingEnabled(bool blending_enabled)
{
    renderer_api_.SetBlendingEnabled(blending_enabled);
}

void RenderCommand::SetLineWidth(float lineWidth)
{
    renderer_api_.SetLineWidth(lineWidth);
}

void RenderCommand::SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height)
{
    renderer_api_.SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return render_stats_;
}

void RenderCommand::NotifyIndexBufferCreated(std::int32_t buffer_size)
{
    render_stats_.index_bufer_memory_allocation += buffer_size;
}

void RenderCommand::NotifyIndexBufferDestroyed(std::int32_t buffer_size)
{
    render_stats_.index_bufer_memory_allocation -= buffer_size;
}

void RenderCommand::NotifyVertexBufferCreated(std::int32_t buffer_size)
{
    render_stats_.vertex_buffer_memory_allocation += buffer_size;
}

void RenderCommand::NotifyVertexBufferDestroyed(std::int32_t buffer_size)
{
    render_stats_.vertex_buffer_memory_allocation -= buffer_size;
}

void RenderCommand::DrawTrianglesInstanced(const VertexArray& vertex_array, size_t num_instances)
{
    renderer_api_.DrawTrianglesInstanced(vertex_array, num_instances);
    render_stats_.num_drawcalls++;
}