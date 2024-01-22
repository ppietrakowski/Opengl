#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"
#include "lights.h"
#include "camera_projection.h"

#include "box.h"

#include <cstdint>

struct Line {
    glm::vec3 start_pos{0.0f};
    glm::vec3 end_pos{0.0f};
};


struct SubmitCommandArgs {
    const Material* material{nullptr};
    int num_indices{0};
    const VertexArray* vertex_array{nullptr};
    glm::mat4 transform = glm::identity<glm::mat4>();

    Shader& GetShader() const {
        return material->GetShader();
    }

    void SetupShader() const {
        material->SetupRenderState();
        material->SetShaderUniforms();
    }
};

class Renderer {
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

    static void DrawDebugBox(const Box& box, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void DrawDebugLine(const Line& line, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void FlushDrawDebug();

    static bool IsVisibleToCamera(glm::vec3 worldspace_position, glm::vec3 bbox_min, glm::vec3 bbox_max);

    static void AddLight(const LightData& light_data);

    static void InitializeDebugDraw(const std::shared_ptr<Shader>& debug_shader);

    static glm::mat4 view_;
    static glm::mat4 projection_;
private:
    static glm::mat4 view_projection_;
    static glm::vec3 camera_position_;
    static std::shared_ptr<Texture2D> default_texture_;

    static void UploadUniforms(Shader& shader, const glm::mat4& transform);
};

FORCE_INLINE std::shared_ptr<Texture2D> Renderer::GetDefaultTexture() {
    return default_texture_;
}