#pragma once

#include "renderer_api.h"
#include <GL/glew.h>

class OpenGlRenderApi : public RendererAPI {
public:
    OpenGlRenderApi();

private:
    void Clear() override;
    void SetClearColor(const RgbaColor& clear_color) override;
    void DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive) override;

    void SetWireframe(bool wireframe_enabled) override;
    bool IsWireframeEnabled() override;
    void SetCullFace(bool cull_face) override;

    bool DoesCullFaces() override;
    void SetBlendingEnabled(bool blending_enabled) override;
    void SetLineWidth(float line_width) override;

    void ClearBufferBindings_Debug() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

private:
    bool cull_enabled_ = false;
    bool wireframe_enabled_ = false;
    bool blending_enabled_ = false;
    RgbaColor clear_color_;
};

