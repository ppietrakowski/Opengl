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

struct Line
{
    glm::vec3 StartPos{0.0f};
    glm::vec3 EndPos{0.0f};
};

struct SubmitCommandArgs
{
    const Material* UsedMaterial{nullptr};
    int NumIndices{0};
    const VertexArray* TargetVertexArray{nullptr};
    glm::mat4 Transform = glm::identity<glm::mat4>();

    Shader& GetShader() const
    {
        return UsedMaterial->GetShader();
    }

    void SetupShader() const
    {
        UsedMaterial->SetupRenderState();
        UsedMaterial->SetShaderUniforms();
    }
};

class Renderer
{
public:
    static void Initialize();
    static void Quit();

    static void UpdateProjection(const CameraProjection& projection);

public:
    static void BeginScene(glm::vec3 cameraPosition, glm::quat cameraRotation);
    static void EndScene();

    static void SubmitTriangles(const SubmitCommandArgs& submitArgs);
    static void SubmitLines(const SubmitCommandArgs& submitArgs);
    static void SubmitPoints(const SubmitCommandArgs& submitArgs);

    static void SubmitSkeleton(const SubmitCommandArgs& submitArgs, std::span<const glm::mat4> transforms);

    static void SubmitMeshInstanced(const SubmitCommandArgs& submitArgs, const UniformBuffer& transformBuffer, int numInstances);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static void DrawDebugBox(const Box& box, const Transform& transform, const glm::vec4& Color = glm::vec4{1, 1, 1, 1});
    static void DrawDebugLine(const Line& line, const Transform& transform, const glm::vec4& Color = glm::vec4{1, 1, 1, 1});
    static void FlushDrawDebug();

    static bool IsVisibleToCamera(glm::vec3 worldspacePosition, glm::vec3 bboxMin, glm::vec3 bboxMax);

    static void AddLight(const LightData& lightData);

    static void InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader);

    static glm::mat4 s_View;
    static glm::mat4 s_Projection;
private:
    static glm::mat4 s_ViewProjection;
    static glm::vec3 s_CameraPosition;
    static std::shared_ptr<Texture2D> s_DefaultTexture;

    static void UploadUniforms(Shader& shader, const glm::mat4& transform);
};

FORCE_INLINE std::shared_ptr<Texture2D> Renderer::GetDefaultTexture()
{
    return s_DefaultTexture;
}