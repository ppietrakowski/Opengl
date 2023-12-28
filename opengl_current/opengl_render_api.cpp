#include "opengl_render_api.h"

OpenGlRenderApi::OpenGlRenderApi() :
    m_ClearColor{0, 0, 0}
{
    glEnable(GL_DEPTH_TEST);
}

void OpenGlRenderApi::Clear()
{
    constexpr GLenum kClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    glClear(kClearFlags);
}

void OpenGlRenderApi::SetClearColor(const RgbaColor& clearColor)
{
    if (m_ClearColor != clearColor)
    {
        glClearColor(clearColor.Red / 255.0f, clearColor.Green / 255.0f, clearColor.Blue / 255.0f, clearColor.Alpha / 255.0f);
        m_ClearColor = clearColor;
    }
}

void OpenGlRenderApi::DrawIndexed(const IVertexArray& vertexArray, uint32_t numIndices, RenderPrimitive renderPrimitive)
{
    vertexArray.Bind();
    glDrawElements(static_cast<GLenum>(renderPrimitive), static_cast<GLsizei>(numIndices), GL_UNSIGNED_INT, nullptr);
}

void OpenGlRenderApi::SetWireframe(bool bWireframeEnabled)
{
    if (bWireframeEnabled != m_bWireframeEnabled)
    {
        if (bWireframeEnabled)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        m_bWireframeEnabled = bWireframeEnabled;
    }
}

bool OpenGlRenderApi::IsWireframeEnabled()
{
    return m_bWireframeEnabled;
}

void OpenGlRenderApi::SetCullFace(bool bCullFace)
{
    if (bCullFace && !DoesCullFaces())
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }
    else if (!bCullFace)
    {
        glDisable(GL_CULL_FACE);
    }

    m_bCullEnabled = bCullFace;
}

bool OpenGlRenderApi::DoesCullFaces()
{
    return m_bCullEnabled;
}

void OpenGlRenderApi::SetBlendingEnabled(bool bBlendingEnabled)
{
    if (bBlendingEnabled && !m_bBlendingEnabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    m_bBlendingEnabled = bBlendingEnabled;
}

void OpenGlRenderApi::SetLineWidth(float lineWidth)
{
    glLineWidth(lineWidth);
}

void OpenGlRenderApi::ClearBufferBindings_Debug()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGlRenderApi::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}
