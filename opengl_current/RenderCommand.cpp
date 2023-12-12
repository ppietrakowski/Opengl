#include "RenderCommand.h"

static bool CullEnabled = false;
static bool WireframeEnabled = false;
static bool BlendingEnabled = false;

void RenderCommand::DrawIndexed(const VertexArray& vertexArray, unsigned int numIndices, ERenderPrimitive renderPrimitive)
{
    vertexArray.Bind();
    glDrawElements(static_cast<GLenum>(renderPrimitive), numIndices, GL_UNSIGNED_INT, NULL);
}

void RenderCommand::SetClearColor(float red, float green, float blue, float alpha)
{
    glClearColor(red, green, blue, alpha);
}

void RenderCommand::Clear(unsigned int clearFlags)
{
    glClear(clearFlags);
}

void RenderCommand::ToggleWireframe()
{
    WireframeEnabled = !WireframeEnabled;

    if (WireframeEnabled)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void RenderCommand::SetWireframe(bool wireframeEnabled)
{
    if (wireframeEnabled != IsWireframeEnabled())
    {
        ToggleWireframe();
    }
}

bool RenderCommand::IsWireframeEnabled()
{
    return WireframeEnabled;
}

void RenderCommand::SetCullFace(bool cullFace)
{
    if (cullFace && !DoesCullFaces())
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }
    else if (!cullFace)
    {
        glDisable(GL_CULL_FACE);
    }

    CullEnabled = cullFace;
}

bool RenderCommand::DoesCullFaces()
{
    return CullEnabled;
}

void RenderCommand::SetBlendingEnabled(bool blendingEnabled)
{
    if (blendingEnabled && !BlendingEnabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    BlendingEnabled = blendingEnabled;
}

