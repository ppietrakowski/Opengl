#include "opengl_render_api.h"

OpenGlRenderApi::OpenGlRenderApi() :
    ClearColor{0, 0, 0}
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
    if (ClearColor != clearColor)
    {
        glClearColor(clearColor.Red / 255.0f, clearColor.Green / 255.0f, clearColor.Blue / 255.0f, clearColor.Alpha / 255.0f);
        ClearColor = clearColor;
    }
}

void OpenGlRenderApi::DrawIndexed(const IndexedDrawData& drawData)
{
    drawData.Bind();
    glDrawElements(static_cast<GLenum>(drawData.DrawPrimitive),
        static_cast<GLsizei>(drawData.NumIndices), GL_UNSIGNED_INT, nullptr);
}

void OpenGlRenderApi::SetWireframe(bool bWireframeEnabled)
{
    if (bWireframeEnabled != bWireframeEnabled)
    {
        if (bWireframeEnabled)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        bWireframeEnabled = bWireframeEnabled;
    }
}

bool OpenGlRenderApi::IsWireframeEnabled()
{
    return bWireframeEnabled;
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

    bCullEnabled = bCullFace;
}

bool OpenGlRenderApi::DoesCullFaces()
{
    return bCullEnabled;
}

void OpenGlRenderApi::SetBlendingEnabled(bool bBlendingEnabled)
{
    if (bBlendingEnabled && !bBlendingEnabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    bBlendingEnabled = bBlendingEnabled;
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

void OpenGlRenderApi::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}
