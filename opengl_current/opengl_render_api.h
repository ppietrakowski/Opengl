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
    void DrawIndexed(const IVertexArray& vertexArray, uint32_t numIndices, RenderPrimitive renderPrimitive) override;

    void SetWireframe(bool bWireframeEnabled) override;
    bool IsWireframeEnabled() override;
    void SetCullFace(bool bCullFace) override;

    bool DoesCullFaces() override;
    void SetBlendingEnabled(bool bBlendingEnabled) override;
    void SetLineWidth(float lineWidth) override;

    void ClearBufferBindings_Debug() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

private:
    bool m_bCullEnabled = false;
    bool m_bWireframeEnabled = false;
    bool m_bBlendingEnabled = false;
    RgbaColor m_ClearColor;
};

