#pragma once

#include "renderer_api.h"
#include <GL/glew.h>

class OpenGlRenderApi : public IRendererAPI {
public:
    OpenGlRenderApi();

private:
    void Clear() override;
    void SetClearColor(const RgbaColor& clear_color) override;
    void DrawIndexed(const IndexedDrawData& draw_data) override;

    void SetWireframe(bool wireframe_enabled) override;
    bool IsWireframeEnabled() override;
    void SetCullFace(bool cull_face) override;

    bool DoesCullFaces() override;
    void SetBlendingEnabled(bool blending_enabled) override;
    void SetLineWidth(float line_width) override;

    void ClearBufferBindings_Debug() override;
    void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) override;

private:
    bool cull_enabled_ = false;
    bool wireframe_enabled_ = false;
    bool blending_enabled_ = false;
    RgbaColor clear_color_;
};

