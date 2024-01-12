#include "renderer_api.h"

#include <GL/glew.h>

void RendererApi::Initialize()
{
    glEnable(GL_DEPTH_TEST);
}

void RendererApi::Clear()
{
    constexpr GLenum ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    glClear(ClearFlags);
}

void RendererApi::SetClearColor(const RgbaColor& clearColor)
{
    if (m_ClearColor != clearColor)
    {
        glClearColor(clearColor.Red / 255.0f, clearColor.Green / 255.0f, clearColor.Blue / 255.0f, clearColor.Alpha / 255.0f);
        m_ClearColor = clearColor;
    }
}

void RendererApi::DrawTriangles(const VertexArray& vertexArray, std::int32_t numIndices)
{
    vertexArray.Bind();
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawTrianglesAdjancency(const VertexArray& vertexArray, std::int32_t numIndices)
{
    vertexArray.Bind();
    glDrawElements(GL_TRIANGLES_ADJACENCY, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawLines(const VertexArray& vertexArray, std::int32_t numIndices)
{
    vertexArray.Bind();
    glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawPoints(const VertexArray& vertexArray, std::int32_t numIndices)
{
    vertexArray.Bind();
    glDrawElements(GL_POINTS, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::SetWireframe(bool bWireframeEnabled)
{
    if (m_bWireframeEnabled != bWireframeEnabled)
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

bool RendererApi::IsWireframeEnabled()
{
    return m_bWireframeEnabled;
}

void RendererApi::SetCullFace(bool bCullFace)
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

bool RendererApi::DoesCullFaces()
{
    return m_bCullEnabled;
}

void RendererApi::SetBlendingEnabled(bool bBlendingEnabled)
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

void RendererApi::SetLineWidth(float lineWidth)
{
    glLineWidth(lineWidth);
}

void RendererApi::ClearBufferBindings_Debug()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RendererApi::SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}
