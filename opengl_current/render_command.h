#pragma once

#include "renderer_api.h"
#include "uniform_buffer.h"
#include <cstdint>

struct RenderStats
{
    std::int32_t num_drawcalls{0};
    std::int64_t delta_frame_time{0};
    std::int32_t index_bufer_memory_allocation{0};
    std::int32_t vertex_buffer_memory_allocation{0};
};

class RenderCommand
{
public:
    RenderCommand() = delete;
    RenderCommand(const RenderCommand&) = delete;

public:

    static void Initialize();
    static void Quit();

    static void ClearBufferBindings_Debug();

    static void DrawTriangles(const VertexArray& vertex_array, std::int32_t num_indices);
    static void DrawTrianglesAdjancency(const VertexArray& vertex_array, std::int32_t num_indices);
    static void DrawLines(const VertexArray& vertex_array, std::int32_t num_indices);
    static void DrawPoints(const VertexArray& vertex_array, std::int32_t num_indices);

    static void BeginScene();
    static void EndScene();
    static void SetClearColor(const RgbaColor& clear_color);
    static void Clear();

    static void SetWireframe(bool wireframe_enabled);
    static bool IsWireframeEnabled();

    static void SetCullFace(bool cull_faces);
    static bool DoesCullFaces();

    static void SetBlendingEnabled(bool blending_enabled);

    static void SetLineWidth(float line_width);
    static void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height);

    static RenderStats GetRenderStats();

    static void NotifyIndexBufferCreated(std::int32_t buffer_size);
    static void NotifyIndexBufferDestroyed(std::int32_t buffer_size);

    static void NotifyVertexBufferCreated(std::int32_t buffer_size);
    static void NotifyVertexBufferDestroyed(std::int32_t buffer_size);

    static void DrawTrianglesInstanced(const VertexArray& vertex_array, std::size_t num_instances);


private:
    static RendererApi renderer_api_;
};

