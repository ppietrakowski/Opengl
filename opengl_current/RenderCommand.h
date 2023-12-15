#pragma once

#include "VertexArray.h"

enum class RenderPrimitive
{
    Points = GL_POINTS,
    Lines = GL_LINES,
    Triangles = GL_TRIANGLES,
    TrianglesAdjancency = GL_TRIANGLES_ADJACENCY
};

class RenderCommand
{
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void DrawIndexed(const VertexArray& vertexArray, std::uint32_t numIndices, RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);
    
    static void SetClearColor(float red, float green, float blue, float alpha = 1.0f);
    static void Clear(std::uint32_t clearFlags);

    static void ToggleWireframe();
    static void SetWireframe(bool wireframeEnabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool cullFace);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool blendingEnabled);
};

