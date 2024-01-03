#pragma once

#include "renderer_api.h"
#include <GL/glew.h>

class OpenGlRenderApi : public IRendererAPI
{
public:
    OpenGlRenderApi();

private:
    void Clear() override;
    void SetClearColor(const RgbaColor& clearColor) override;
    void DrawIndexed(const IndexedDrawData& drawData) override;

    void SetWireframe(bool bWireframeEnabled) override;
    bool IsWireframeEnabled() override;
    void SetCullFace(bool bCullFace) override;

    bool DoesCullFaces() override;
    void SetBlendingEnabled(bool bBlendingEnabled) override;
    void SetLineWidth(float lineWidth) override;

    void ClearBufferBindings_Debug() override;
    void SetViewport(int32_t x, int32_t y, int32_t width, int32_t height) override;

private:
    bool bCullEnabled = false;
    bool bWireframeEnabled = false;
    bool bBlendingEnabled = false;
    RgbaColor ClearColor;
};

