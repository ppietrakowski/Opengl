#include "renderer_api.h"
#include "error_macros.h"
#include "logging.h"

#include <GL/glew.h>

static void OpenGlErrorCallback(
    unsigned source,
    unsigned type,
    unsigned id,
    unsigned severity,
    int32_t length,
    const char* message,
    const void* userParam)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         ELOG_ERROR(LOG_RENDERER, message); return;
    case GL_DEBUG_SEVERITY_MEDIUM:       ELOG_ERROR(LOG_RENDERER, message); return;
    case GL_DEBUG_SEVERITY_LOW:          ELOG_WARNING(LOG_RENDERER, message); return;
    case GL_DEBUG_SEVERITY_NOTIFICATION: ELOG_VERBOSE(LOG_RENDERER, message); return;
    }
}

void RendererApi::Initialize()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    SetCullFace(true);

#if defined(DEBUG) || defined(_DEBUG)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    constexpr const void* const userData = nullptr;
    glDebugMessageCallback(&OpenGlErrorCallback, userData);

    constexpr GLenum sourceOfDebugMessage = GL_DONT_CARE;
    constexpr GLenum typeOfDebugMessage = GL_DONT_CARE;
    constexpr GLenum severityOfDebugMessage = GL_DEBUG_SEVERITY_NOTIFICATION;
    constexpr GLsizei numMessageIds = 0;
    constexpr const GLuint* const messageIds = nullptr;
    constexpr GLboolean selectedMessagesEnabled = GL_FALSE;

    glDebugMessageControl(sourceOfDebugMessage, typeOfDebugMessage, 
        severityOfDebugMessage, numMessageIds, messageIds, selectedMessagesEnabled);
#endif
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

static FORCE_INLINE void DrawIndexedUsingGlPrimitives(const VertexArray& vertexArray, int32_t numIndices, GLenum primitiveType)
{
    ASSERT(numIndices >= 0);

    vertexArray.Bind();
    glDrawElements(primitiveType, numIndices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawIndexed(const VertexArray& vertexArray, int32_t numIndices)
{
    DrawIndexedUsingGlPrimitives(vertexArray, numIndices, GL_TRIANGLES);
}

void RendererApi::DrawArrays(const VertexArray& vertexArray, int32_t numVertices)
{
    vertexArray.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

void RendererApi::DrawLines(const VertexArray& vertexArray, int32_t numIndices)
{
    DrawIndexedUsingGlPrimitives(vertexArray, numIndices, GL_LINES);
}

void RendererApi::DrawIndexedInstanced(const VertexArray& vertexArray, int32_t numInstances)
{
    ASSERT(numInstances >= 0);

    vertexArray.Bind();
    glDrawElementsInstanced(GL_TRIANGLES, vertexArray.GetNumIndices(),
        GL_UNSIGNED_INT, nullptr, numInstances);
}

void RendererApi::SetCullFace(bool bCullFaces)
{
    if (bCullFaces)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    m_bCullFaces = bCullFaces;
}

bool RendererApi::DoesCullFaces() const
{
    return m_bCullFaces;
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

void RendererApi::SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}

void RendererApi::SetDepthFunc(DepthFunction depthFunction)
{
    GLenum functions[] = {GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL};

    glDepthFunc(functions[(size_t)depthFunction]);
}

void RendererApi::SetDepthEnabled(bool bDepthEnabled)
{
    glDepthMask(static_cast<GLboolean>(bDepthEnabled));
}
