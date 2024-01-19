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

    CameraProjection(uint32_t width, uint32_t height, float fov = 45.0f, float z_near = 0.1f, float z_far = 1000.0f) :
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

struct SubmitCommandArgs
{
    const Material* material;
    int num_indices{0};
    const VertexArray* vertex_array;
    glm::mat4 transform = glm::mat4{1.0f};

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

    static void SubmitTriangles(const SubmitCommandArgs& submit_args);
    static void SubmitLines(const SubmitCommandArgs& submit_args);
    static void SubmitPoints(const SubmitCommandArgs& submit_args);

    static void SubmitSkeleton(const SubmitCommandArgs& submit_args, std::span<const glm::mat4> transforms);

    static void SubmitMeshInstanced(const SubmitCommandArgs& submit_args, const UniformBuffer& transform_buffer, int num_instances);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static void DrawDebugBox(glm::vec3 box_min, glm::vec3 box_max, const Transform& transform, const glm::vec4& color = glm::vec4{0, 0, 0, 1});
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