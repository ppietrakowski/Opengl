#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"

#include <cstdint>

struct CameraProjection
{
    float fov{45.0f};
    float aspect_ratio{0.0f};
    float width;
    float height;
    float z_near{0.1f};
    float z_far{1000.0f};

    CameraProjection(std::int32_t width, std::int32_t height, float fov = 45.0f, float z_near = 0.1f, float z_far = 1000.0f) :
        width((float)width),
        height((float)height),
        fov(fov),
        z_near(z_near),
        z_far(z_far)
    {
        aspect_ratio = this->width / this->height;
    }

    CameraProjection() = default;
    CameraProjection(const CameraProjection&) = default;
    CameraProjection& operator=(const CameraProjection&) = default;
};

struct InstancingSubmission
{
    const Material* material{nullptr};
    const VertexArray* vertex_array{nullptr};
    const UniformBuffer* transform_buffer{nullptr};
    std::size_t num_instances{0};
    glm::mat4 transform{1.0f};

    Shader& GetShader() const
    {
        return material->GetShader();
    }

    void SetupShader() const
    {
        material->SetupRenderState();
        material->SetShaderUniforms();
    }
};

class Renderer
{
public:
    static void Initialize();
    static void Quit();

    static void UpdateProjection(const CameraProjection& projection);

public:
    static void BeginScene(glm::vec3 camera_pos, glm::quat camera_rotation);
    static void EndScene();

    static void SubmitTriangles(const Material& material,
        std::int32_t num_indices,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{1.0f});

    static void SubmitTriangles(const Material& material,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{1.0f});

    static void SubmitLines(const Material& material,
        std::int32_t num_indices,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{1.0f});

    static void SubmitPoints(const Material& material,
        std::int32_t num_indices,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{1.0f});

    static void SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms,
        const VertexArray& vertex_array,
        const glm::mat4& transform = glm::mat4{1.0f});

    static void SubmitMeshInstanced(const InstancingSubmission& instance_submission);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static void DrawDebugBox(glm::vec3 box_min, glm::vec3 box_max, const Transform& transform);
    static void DrawDebugBox(glm::vec3 box_min, glm::vec3 box_max, const Transform& transform, const glm::vec4& color);
    static void FlushDrawDebug(Material& shader);

    static bool IsVisibleToCamera(glm::vec3 worldspace_position, glm::vec3 bbox_min, glm::vec3 bbox_max);

private:
    static glm::mat4 view_;
    static glm::mat4 projection_;
    static glm::mat4 view_projection_;
    static glm::vec3 camera_position_;
    static std::shared_ptr<Texture2D> default_texture_;

    static void UploadUniforms(Shader& shader, const glm::mat4& transform);
};

FORCE_INLINE std::shared_ptr<Texture2D> Renderer::GetDefaultTexture()
{
    return default_texture_;
}