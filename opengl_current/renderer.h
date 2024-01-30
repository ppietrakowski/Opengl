#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"
#include "lights.h"
#include "camera_projection.h"
#include "submit_args.h"
#include "box.h"
#include "viewport.h"

#include "static_mesh.h"
#include "skeletal_mesh.h"

#include <cstdint>

struct RendererData
{
    glm::mat4 ViewMatrix = glm::identity<glm::mat4>();
    glm::mat4 ProjectionViewMatrix = glm::identity<glm::mat4>();
    glm::mat4 ProjectionMatrix = glm::identity<glm::mat4>();
    glm::vec3 CameraPosition{0, 0, 0};
    CameraProjection Projection;
};

class Renderer
{
    friend class Game;

public:
    static void UpdateProjection(const CameraProjection& projection);

    static void BeginScene(glm::vec3 cameraPosition, glm::quat cameraRotation, const std::vector<LightData>& lights);
    static void EndScene();

    static void Submit(const StaticMeshEntry& meshEntry, const glm::mat4& transform);
    static void SubmitSkeleton(const SkeletalMesh& skeletalMesh, const glm::mat4& transform, std::span<const glm::mat4> boneTransforms);

    static void SubmitMeshInstanced(const StaticMeshEntry& mesh, const Material& material, const UniformBuffer& buffer, int32_t numInstances, const glm::mat4& transform);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static glm::mat4 GetViewMatrix()
    {
        return s_RendererData.ViewMatrix;
    }

    static glm::mat4 GetProjectionMatrix()
    {
        return s_RendererData.ProjectionMatrix;
    }

    static glm::mat4 GetProjectionViewMatrix()
    {
        return s_RendererData.ProjectionViewMatrix;
    }

    static Viewport GetViewport()
    {
        return Viewport{glm::vec2(0, 0), glm::vec2(s_RendererData.Projection.Width, s_RendererData.Projection.Height)};
    }

private:
    static RendererData s_RendererData;
    static std::shared_ptr<Texture2D> s_DefaultTexture;

private:
    static void Initialize();
    static void Quit();
    static void StartSubmiting(const Material& material, const glm::mat4& transform);
    static void UploadUniforms(const std::shared_ptr<Shader>& shader, const glm::mat4& transform, uint32_t cubeMapTextureUnit);
};

FORCE_INLINE std::shared_ptr<Texture2D> Renderer::GetDefaultTexture()
{
    return s_DefaultTexture;
}