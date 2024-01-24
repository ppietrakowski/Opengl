#include "renderer_api.h"
#include "error_macros.h"

#include <GL/glew.h>

void RendererApi::Initialize()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
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

void RendererApi::DrawTriangles(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(numIndices >= 0);

    vertexArray.Bind();
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawTrianglesArrays(const VertexArray& vertexArray, int numVertices)
{
    vertexArray.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

void RendererApi::DrawTrianglesAdjancency(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(numIndices >= 0);

    vertexArray.Bind();
    glDrawElements(GL_TRIANGLES_ADJACENCY, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawLines(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(numIndices >= 0);

    vertexArray.Bind();
    glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawPoints(const VertexArray& vertexArray, int numIndices)
{
    ASSERT(numIndices >= 0);

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

bool RendererApi::IsWireframeEnabled() const
{
    return m_bWireframeEnabled;
}

void RendererApi::SetCullFace(bool bCullFaces)
{
    if (bCullFaces && !DoesCullFaces())
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }
    else if (!bCullFaces)
    {
        glDisable(GL_CULL_FACE);
    }

    m_bCullFaces = bCullFaces;
}

void RendererApi::UpdateCullFace(bool bUseClockwise)
{
    glFrontFace(bUseClockwise ? GL_CW : GL_CCW);
}

bool RendererApi::DoesCullFaces() const
{
    return m_bCullFaces;
}

void RendererApi::SetBlendingEnabled(bool bBlendingEnabled)
{
    this->m_bBlendingEnabled = bBlendingEnabled;
}

void RendererApi::SetLineWidth(float lineWidth)
{
    glLineWidth(lineWidth);
}

void RendererApi::SetDepthEnabled(bool bEnabled)
{
    if (bEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void RendererApi::ClearBufferBindings_Debug()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RendererApi::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}

void RendererApi::DrawTrianglesInstanced(const VertexArray& vertexArray, int numInstances)
{
    ASSERT(numInstances >= 0);

    vertexArray.Bind();
    glDrawElementsInstanced(GL_TRIANGLES, vertexArray.GetNumIndices(),
        GL_UNSIGNED_INT, nullptr, numInstances);
}


void RendererApi::SetDepthFunc(DepthFunction depthFunction)
{
    GLenum functions[] = {GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL};

    glDepthFunc(functions[(size_t)depthFunction]);
}
