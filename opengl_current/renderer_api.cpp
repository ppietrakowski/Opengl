#include "renderer_api.h"
#include "error_macros.h"

#include <GL/glew.h>

void RendererApi::Initialize() {
    glEnable(GL_DEPTH_TEST);
}

void RendererApi::Clear() {
    constexpr GLenum kClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    glClear(kClearFlags);
}

void RendererApi::SetClearColor(const RgbaColor& clear_color) {
    if (clear_color_ != clear_color) {
        glClearColor(clear_color.red / 255.0f, clear_color.green / 255.0f, clear_color.blue / 255.0f, clear_color.alpha / 255.0f);
        clear_color_ = clear_color;
    }
}

void RendererApi::DrawTriangles(const VertexArray& vertex_array, int num_indices) {
    ASSERT(num_indices >= 0);

    vertex_array.Bind();
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawTrianglesAdjancency(const VertexArray& vertex_array, int num_indices) {
    ASSERT(num_indices >= 0);

    vertex_array.Bind();
    glDrawElements(GL_TRIANGLES_ADJACENCY, num_indices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawLines(const VertexArray& vertex_array, int num_indices) {
    ASSERT(num_indices >= 0);

    vertex_array.Bind();
    glDrawElements(GL_LINES, num_indices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::DrawPoints(const VertexArray& vertex_array, int num_indices) {
    ASSERT(num_indices >= 0);

    vertex_array.Bind();
    glDrawElements(GL_POINTS, num_indices, GL_UNSIGNED_INT, nullptr);
}

void RendererApi::SetWireframe(bool wireframe_enabled) {
    if (wireframe_enabled_ != wireframe_enabled) {
        if (wireframe_enabled) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        wireframe_enabled_ = wireframe_enabled;
    }
}

bool RendererApi::IsWireframeEnabled() const {
    return wireframe_enabled_;
}

void RendererApi::SetCullFace(bool cull_faces) {
    if (cull_faces && !DoesCullFaces()) {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    } else if (!cull_faces) {
        glDisable(GL_CULL_FACE);
    }

    cull_enabled_ = cull_faces;
}

bool RendererApi::DoesCullFaces() const {
    return cull_enabled_;
}

void RendererApi::SetBlendingEnabled(bool blending_enabled) {
    if (blending_enabled && !blending_enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }

    blending_enabled = blending_enabled;
}

void RendererApi::SetLineWidth(float lineWidth) {
    glLineWidth(lineWidth);
}

void RendererApi::ClearBufferBindings_Debug() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RendererApi::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}

void RendererApi::DrawTrianglesInstanced(const VertexArray& vertex_array, int num_instances) {
    ASSERT(num_instances >= 0);

    vertex_array.Bind();
    glDrawElementsInstanced(GL_TRIANGLES, vertex_array.GetNumIndices(),
        GL_UNSIGNED_INT, nullptr, num_instances);
}
