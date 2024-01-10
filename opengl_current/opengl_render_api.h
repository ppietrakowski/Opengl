#pragma once

#include "renderer_api.h"
#include <GL/glew.h>

class OpenGlRenderApi : public RendererAPI
{
public:
    OpenGlRenderApi();

private:
    void Clear() override;
    void SetClearColor(const RgbaColor& clearColor) override;
    void DrawTriangles(const VertexArray& vertexArray, std::int32_t numIndices) override;
    void DrawTrianglesAdjancency(const VertexArray& vertexArray, std::int32_t numIndices) override;
    void DrawLines(const VertexArray& vertexArray, std::int32_t numIndices) override;
    void DrawPoints(const VertexArray& vertexArray, std::int32_t numIndices) override;

    void SetWireframe(bool bWireframeEnabled) override;
    bool IsWireframeEnabled() override;
    void SetCullFace(bool bCullFace) override;

    bool DoesCullFaces() override;
    void SetBlendingEnabled(bool bBlendingEnabled) override;
    void SetLineWidth(float lineWidth) override;

    void ClearBufferBindings_Debug() override;
    void SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) override;

private:
    bool m_bCullEnabled = false;
    bool m_bWireframeEnabled = false;
    bool m_bBlendingEnabled = false;
    RgbaColor m_ClearColor;
};

