#pragma once

#include "renderer_api.h"
#include <cstdint>

struct RenderStats {
    uint32_t num_drawcalls{ 0 };
    uint32_t num_triangles{ 0 };
    uint64_t delta_frame_nanoseconds{ 0 };
};

class RenderCommand {
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void Initialize();
    static void Quit();

    static void ClearBufferBindings_Debug();

    static void DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive = RenderPrimitive::kTriangles);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clear_color);
    static void Clear();

    static void SetWireframe(bool wireframe_enabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool cullFace);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool blending_enabled);

    static void SetLineWidth(float line_width);

    static RenderStats GetRenderStats();

private:
    static RendererAPI* renderer_api_;
};

