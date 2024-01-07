#pragma once

#include "renderer_api.h"
#include <cstdint>

struct RenderStats {
    std::int32_t num_drawcalls{0};
    std::int32_t num_drawn_triangles{0};
    std::int64_t delta_frame_nanoseconds{0};
};

class RenderCommand {
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void Initialize();
    static void Quit();

    static void ClearBufferBindings_Debug();

    static void DrawIndexed(const IndexedDrawData& draw_data);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clear_color);
    static void Clear();

    static void SetWireframe(bool wireframe_enabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool cull_face);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool blending_enabled);

    static void SetLineWidth(float line_width);
    static void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height);

    static RenderStats GetRenderStats();

private:
    static IRendererAPI* renderer_api_;
};

