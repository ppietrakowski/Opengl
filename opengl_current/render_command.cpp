#include "render_command.h"
#include "error_macros.h"

#include <chrono>

static RenderStats render_stats_;
static std::chrono::nanoseconds s_StartTimestamp = std::chrono::nanoseconds::zero();
RendererApi RenderCommand::renderer_api_{};
static bool s_bRenderCommandInitialized = false;

FORCE_INLINE int GetNumIndices(int num_indices, const VertexArray& vertex_array)
{
    return num_indices == 0 ? vertex_array.GetNumIndices() : num_indices;
}

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

void RenderCommand::DrawTriangles(const VertexArray& vertex_array, int num_indices)
{
    ASSERT(s_bRenderCommandInitialized);
    renderer_api_.DrawTriangles(vertex_array, GetNumIndices(num_indices, vertex_array));
    render_stats_.num_drawcalls++;
}

void RenderCommand::DrawTrianglesAdjancency(const VertexArray& vertex_array, int num_indices)
{
    ASSERT(s_bRenderCommandInitialized);

    renderer_api_.DrawTrianglesAdjancency(vertex_array, GetNumIndices(num_indices, vertex_array));
    render_stats_.num_drawcalls++;
}
void RenderCommand::DrawLines(const VertexArray& vertex_array, int num_indices)
{
    ASSERT(s_bRenderCommandInitialized);

    renderer_api_.DrawLines(vertex_array, GetNumIndices(num_indices, vertex_array));
    render_stats_.num_drawcalls++;
}
void RenderCommand::DrawPoints(const VertexArray& vertex_array, int num_indices)
{
    ASSERT(s_bRenderCommandInitialized);

    renderer_api_.DrawPoints(vertex_array, GetNumIndices(num_indices, vertex_array));
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

void RenderCommand::SetViewport(int x, int y, int width, int height)
{
    renderer_api_.SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats()
{
    return render_stats_;
}

void RenderCommand::NotifyIndexBufferCreated(int buffer_size)
{
    render_stats_.index_bufer_memory_allocation += buffer_size;
}

void RenderCommand::NotifyIndexBufferDestroyed(int buffer_size)
{
    render_stats_.index_bufer_memory_allocation -= buffer_size;
}

void RenderCommand::NotifyVertexBufferCreated(int buffer_size)
{
    render_stats_.vertex_buffer_memory_allocation += buffer_size;
}

void RenderCommand::NotifyVertexBufferDestroyed(int buffer_size)
{
    render_stats_.vertex_buffer_memory_allocation -= buffer_size;
}

void RenderCommand::DrawTrianglesInstanced(const VertexArray& vertex_array, int num_instances)
{
    renderer_api_.DrawTrianglesInstanced(vertex_array, num_instances);
    render_stats_.num_drawcalls++;
}