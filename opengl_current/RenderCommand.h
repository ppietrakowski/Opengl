#pragma once

#include "VertexArray.h"

enum class RenderPrimitive
{
    kPoints = GL_POINTS,
    kLines = GL_LINES,
    kTriangles = GL_TRIANGLES,
    kTrianglesAdjancency = GL_TRIANGLES_ADJACENCY
};

class RenderCommand
{
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive = RenderPrimitive::kTriangles);
    
    static void SetClearColor(float red, float green, float blue, float alpha = 1.0f);
    static void Clear(uint32_t clear_flags);

    static void ToggleWireframe();
    static void SetWireframe(bool wireframe_enabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool cullFace);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool blending_enabled);

    static void SetLineWidth(float line_width);
};

