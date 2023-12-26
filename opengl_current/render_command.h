#pragma once

#include "vertex_array.h"
#include <cstdint>

/* Render primitives, which value is same as openGL equivalents*/
enum class RenderPrimitive {
    kPoints = 0x0000, // GL_POINTS
    kLines = 0x0001, // GL_LINES
    kTriangles = 0x0004, // GL_TRIANGLES
    kTrianglesAdjancency = 0x000C // GL_TRIANGLES_ADJANCENCY
};

struct RenderStats {
    uint32_t num_drawcalls{ 0 };
    uint64_t delta_frame_nanoseconds{ 0 };
};

class RenderCommand {
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void Initialize();
    static void ClearBufferBinding();

    static void DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive = RenderPrimitive::kTriangles);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(float red, float green, float blue, float alpha = 1.0f);
    static void Clear(uint32_t clear_flags);

    static void ToggleWireframe();
    static void SetWireframe(bool wireframe_enabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool cullFace);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool blending_enabled);

    static void SetLineWidth(float line_width);

    static RenderStats GetRenderStats();
};

