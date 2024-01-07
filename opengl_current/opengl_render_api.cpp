#include "opengl_render_api.h"

OpenGlRenderApi::OpenGlRenderApi() :
    clear_color_{0, 0, 0}
{
    glEnable(GL_DEPTH_TEST);
}

void OpenGlRenderApi::Clear()
{
    constexpr GLenum kClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    glClear(kClearFlags);
}

void OpenGlRenderApi::SetClearColor(const RgbaColor& clear_color)
{
    if (clear_color_ != clear_color)
    {
        glClearColor(clear_color.red / 255.0f, clear_color.green / 255.0f, clear_color.blue / 255.0f, clear_color.alpha / 255.0f);
        clear_color_ = clear_color;
    }
}

void OpenGlRenderApi::DrawIndexed(const IndexedDrawData& draw_data)
{
    draw_data.Bind();
    glDrawElements(static_cast<GLenum>(draw_data.draw_primitive),
        static_cast<GLsizei>(draw_data.num_indices), GL_UNSIGNED_INT, nullptr);
}

void OpenGlRenderApi::SetWireframe(bool wireframe_enabled)
{
    if (wireframe_enabled != wireframe_enabled)
    {
        if (wireframe_enabled)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        wireframe_enabled_ = wireframe_enabled;
    }
}

bool OpenGlRenderApi::IsWireframeEnabled()
{
    return wireframe_enabled_;
}

void OpenGlRenderApi::SetCullFace(bool cull_face)
{
    if (cull_face && !DoesCullFaces())
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }
    else if (!cull_face)
    {
        glDisable(GL_CULL_FACE);
    }

    cull_enabled_ = cull_face;
}

bool OpenGlRenderApi::DoesCullFaces()
{
    return cull_enabled_;
}

void OpenGlRenderApi::SetBlendingEnabled(bool blending_enabled)
{
    if (blending_enabled && !blending_enabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    blending_enabled = blending_enabled;
}

void OpenGlRenderApi::SetLineWidth(float line_width)
{
    glLineWidth(line_width);
}

void OpenGlRenderApi::ClearBufferBindings_Debug()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGlRenderApi::SetViewport(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}
