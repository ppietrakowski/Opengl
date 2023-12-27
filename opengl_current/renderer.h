#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"

#include <cstdint>

class Renderer {
public:
    static void Initialize();
    static void Quit();

    static void UpdateProjection(float width, float height, float fov, float z_near = 0.1f, float z_far = 1000.0f);

public:
    static void BeginScene(const glm::mat4& view, glm::vec3 camera_position);
    static void EndScene();

    static void Submit(const Material& material,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{ 1.0f },
        RenderPrimitive render_primitive = RenderPrimitive::kTriangles);

    static void SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, uint32_t num_indices,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{ 1.0f },
        RenderPrimitive render_primitive = RenderPrimitive::kTriangles);


    static void Submit(Shader& shader,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{ 1.0f },
        RenderPrimitive render_primitive = RenderPrimitive::kTriangles);

    static void Submit(Shader& shader,
        uint32_t num_indices, const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{ 1.0f }, RenderPrimitive render_primitive = RenderPrimitive::kTriangles);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static void DrawDebugBox(glm::vec3 box_min, glm::vec3 box_max, const glm::mat4& transform);
    static void FlushDrawDebug(Shader& shader);

private:
    static glm::mat4 view_;
    static glm::mat4 projection_;
    static glm::mat4 projection_view_;
    static glm::vec3 camera_position_;
    static std::shared_ptr<Texture2D> default_texture_;

    static void UploadUniforms(Shader& shader, const glm::mat4& transform);
};

inline std::shared_ptr<Texture2D> Renderer::GetDefaultTexture() {
    return default_texture_;
}