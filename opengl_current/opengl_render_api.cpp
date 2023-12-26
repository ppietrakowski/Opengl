#include "opengl_render_api.h"

OpenGlRenderApi::OpenGlRenderApi() :
    clear_color_{ 0, 0, 0 } {
    glEnable(GL_DEPTH_TEST);
}

void OpenGlRenderApi::Clear() {
    constexpr GLenum kClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    glClear(kClearFlags);
}

void OpenGlRenderApi::SetClearColor(const RgbaColor& clear_color) {
    if (clear_color_ != clear_color) {
        glClearColor(clear_color.red / 255.0f, clear_color.green / 255.0f, clear_color.blue / 255.0f, clear_color.alpha / 255.0f);
        clear_color_ = clear_color;
    }
}

void OpenGlRenderApi::DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive) {
    vertex_array.Bind();
    glDrawElements(static_cast<GLenum>(render_primitive), static_cast<GLsizei>(num_indices), GL_UNSIGNED_INT, nullptr);
}

void OpenGlRenderApi::SetWireframe(bool wireframe_enabled) {
    if (wireframe_enabled != wireframe_enabled_) {
        if (wireframe_enabled) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        wireframe_enabled_ = wireframe_enabled;
    }
}

bool OpenGlRenderApi::IsWireframeEnabled() {
    return wireframe_enabled_;
}

void OpenGlRenderApi::SetCullFace(bool cull_face) {
    if (cull_face && !DoesCullFaces()) {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    } else if (!cull_face) {
        glDisable(GL_CULL_FACE);
    }

    cull_enabled_ = cull_face;
}

bool OpenGlRenderApi::DoesCullFaces() {
    return cull_enabled_;
}

void OpenGlRenderApi::SetBlendingEnabled(bool blending_enabled) {
    if (blending_enabled && !blending_enabled_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }

    blending_enabled_ = blending_enabled;
}

void OpenGlRenderApi::SetLineWidth(float line_width) {
    glLineWidth(line_width);
}

void OpenGlRenderApi::ClearBufferBindings_Debug() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
