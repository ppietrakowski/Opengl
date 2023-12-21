#include "RenderCommand.h"

static bool cull_enabled_ = false;
static bool wireframe_enabled_ = false;
static bool blending_enabled_ = false;

void RenderCommand::DrawIndexed(const VertexArray& vertex_array, uint32_t num_indices, RenderPrimitive render_primitive) {
    vertex_array.Bind();
    glDrawElements(static_cast<GLenum>(render_primitive), static_cast<GLsizei>(num_indices), GL_UNSIGNED_INT, nullptr);
}

void RenderCommand::SetClearColor(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
}

void RenderCommand::Clear(uint32_t clear_flags) {
    glClear(clear_flags);
}

void RenderCommand::ToggleWireframe() {
    wireframe_enabled_ = !wireframe_enabled_;

    if (wireframe_enabled_) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void RenderCommand::SetWireframe(bool wireframe_enabled) {
    if (wireframe_enabled != IsWireframeEnabled()) {
        ToggleWireframe();
    }
}

bool RenderCommand::IsWireframeEnabled() {
    return wireframe_enabled_;
}

void RenderCommand::SetCullFace(bool cull_face) {
    if (cull_face && !DoesCullFaces()) {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    } else if (!cull_face) {
        glDisable(GL_CULL_FACE);
    }

    cull_enabled_ = cull_face;
}

bool RenderCommand::DoesCullFaces() {
    return cull_enabled_;
}

void RenderCommand::SetBlendingEnabled(bool blending_enabled) {
    if (blending_enabled && !blending_enabled_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }

    blending_enabled_ = blending_enabled;
}

void RenderCommand::SetLineWidth(float line_width) {
    glLineWidth(line_width);
}

