#include "render_command.h"
#include "opengl_render_api.h"

#include <chrono>

static RenderStats render_stats_;
static std::chrono::nanoseconds start_timestamp_ = std::chrono::nanoseconds::zero();
RendererAPI* RenderCommand::renderer_api_ = nullptr;

void RenderCommand::Initialize() {
    renderer_api_ = new OpenGlRenderApi();
}

void RenderCommand::Quit() {
    delete renderer_api_;
    renderer_api_ = nullptr;
}

void RenderCommand::ClearBufferBindings_Debug() {
    renderer_api_->ClearBufferBindings_Debug();
}

void RenderCommand::DrawIndexed(const IndexedDrawData& draw_data) {
    renderer_api_->DrawIndexed(draw_data);
    render_stats_.num_drawcalls++;
    render_stats_.num_drawn_triangles += (draw_data.num_indices / 3);
}

void RenderCommand::BeginScene() {
    render_stats_.num_drawcalls = 0;
    render_stats_.num_drawn_triangles = 0;
}

void RenderCommand::EndScene() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    render_stats_.delta_frame_nanoseconds = (now - start_timestamp_).count();
    start_timestamp_ = now;
}

void RenderCommand::SetClearColor(const RgbaColor& clear_color) {
    renderer_api_->SetClearColor(clear_color);
}

void RenderCommand::Clear() {
    renderer_api_->Clear();
}

void RenderCommand::SetWireframe(bool wireframe_enabled) {
    renderer_api_->SetWireframe(wireframe_enabled);
}

bool RenderCommand::IsWireframeEnabled() {
    return renderer_api_->IsWireframeEnabled();
}

void RenderCommand::SetCullFace(bool cull_face) {
    renderer_api_->SetCullFace(cull_face);
}

bool RenderCommand::DoesCullFaces() {
    return renderer_api_->DoesCullFaces();
}

void RenderCommand::SetBlendingEnabled(bool blending_enabled) {
    renderer_api_->SetBlendingEnabled(blending_enabled);
}

void RenderCommand::SetLineWidth(float line_width) {
    renderer_api_->SetLineWidth(line_width);
}

void RenderCommand::SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) {
    renderer_api_->SetViewport(x, y, width, height);
}

RenderStats RenderCommand::GetRenderStats() {
    return render_stats_;
}

