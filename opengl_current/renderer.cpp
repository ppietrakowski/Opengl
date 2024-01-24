#include "renderer.h"
#include "error_macros.h"
#include "render_command.h"
#include "renderer_2d.h"
#include "skybox.h"

#include "debug.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

#include <GL/glew.h>

RendererData Renderer::s_RendererData{};
std::shared_ptr<Texture2D> Renderer::s_DefaultTexture;

static constexpr RgbColor Black{0, 0, 0};
static constexpr RgbColor Magenta{255, 0, 255};

struct LightBuffer
{
    UniformBuffer Buffer;
    int ActualNumLights{0};

    LightBuffer(int num_lights) :
        Buffer(num_lights * sizeof(LightData))
    {
    }

    void AddLight(const LightData& lightData)
    {
        Buffer.UpdateBuffer(&lightData, sizeof(LightData), ActualNumLights * sizeof(LightData));
        ActualNumLights++;
    }

    void Clear()
    {
        ActualNumLights = 0;
    }

    void BindBuffer(Shader& shader, const std::string& bindingName) const
    {
        shader.BindUniformBuffer(shader.GetUniformBlockIndex(bindingName), Buffer);
    }
};

static LightBuffer* s_LightBuffer = nullptr;

void Renderer::Initialize()
{
    // array of checkerboard with black and magenta
    RgbColor colors[4][4] =
    {
        {Black, Black, Magenta, Magenta},
        {Black, Black, Magenta, Magenta},
        {Magenta, Magenta, Black, Black},
        {Magenta, Magenta, Black, Black}
    };

    int colorsWidth = 4;
    int colorsHeight = 4;

    s_DefaultTexture = std::make_shared<Texture2D>(colors, TextureSpecification{colorsWidth, colorsHeight, TextureFormat::Rgb});
    RenderCommand::Initialize();

    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);

    s_LightBuffer = new LightBuffer(32);
    Renderer2D::Initialize();
}

void Renderer::Quit()
{
    SafeDelete(s_LightBuffer);
    
    s_DefaultTexture.reset();
    Debug::Quit();
    Renderer2D::Quit();
    RenderCommand::Quit();
}

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    s_RendererData.ProjectionMatrix = glm::perspective(glm::radians(projection.Fov), projection.AspectRatio, projection.ZNear, projection.ZFar);
    Renderer2D::UpdateProjection(projection);
}

void Renderer::BeginScene(glm::vec3 cameraPosition, glm::quat cameraRotation, const std::vector<LightData>& lights)
{
    RenderCommand::BeginScene();
    s_RendererData.ViewMatrix = glm::inverse(glm::translate(glm::identity<glm::mat4>(), cameraPosition) * glm::mat4_cast(cameraRotation));
    s_RendererData.ProjectionViewMatrix = s_RendererData.ProjectionMatrix * s_RendererData.ViewMatrix;
    s_RendererData.CameraPosition = cameraPosition;

    ASSERT(lights.size() <= 32);

    for (const LightData& lightData : lights)
    {
        s_LightBuffer->AddLight(lightData);
    }

    Debug::BeginScene(s_RendererData.ProjectionViewMatrix);
}

void Renderer::EndScene()
{
    Debug::FlushDrawDebug();
    RenderCommand::SetLineWidth(1);
    s_LightBuffer->Clear();
    RenderCommand::EndScene();
}

void Renderer::Submit(const SubmitCommandArgs& submitArgs)
{
    ASSERT(submitArgs.NumIndices <= submitArgs.TargetVertexArray->GetNumIndices());

    submitArgs.SetupShader();
    std::shared_ptr<Shader> shader = submitArgs.GetShader();
    shader->Use();

    uint32_t textureUnit = submitArgs.UsedMaterial->GetNumTextures();
    UploadUniforms(shader, submitArgs.Transform, textureUnit);
    submitArgs.ApplyMaterialUniforms();

    RenderCommand::DrawIndexed(submitArgs.TargetVertexArray, submitArgs.NumIndices);
}

void Renderer::SubmitSkeleton(const SubmitCommandArgs& submitArgs, std::span<const glm::mat4> transforms)
{
    ASSERT(submitArgs.NumIndices <= submitArgs.TargetVertexArray->GetNumIndices());

    submitArgs.SetupShader();
    std::shared_ptr<Shader> shader = submitArgs.GetShader();
    shader->Use();

    uint32_t textureUnit = submitArgs.UsedMaterial->GetNumTextures();
    UploadUniforms(shader, submitArgs.Transform, textureUnit);
    submitArgs.ApplyMaterialUniforms();
    shader->SetUniformMat4Array("u_bone_transforms", transforms);

    RenderCommand::DrawIndexed(submitArgs.TargetVertexArray, submitArgs.NumIndices);
}

void Renderer::SubmitMeshInstanced(const InstancedDrawArgs& instancedDrawArgs)
{
    const SubmitCommandArgs& submitArgs = instancedDrawArgs.SubmitArgs;

    submitArgs.SetupShader();
    std::shared_ptr<Shader> shader = submitArgs.GetShader();
    shader->Use();

    uint32_t textureUnit = submitArgs.UsedMaterial->GetNumTextures();
    UploadUniforms(shader, submitArgs.Transform, textureUnit);
    submitArgs.ApplyMaterialUniforms();;

    shader->BindUniformBuffer(shader->GetUniformBlockIndex("Transforms"), *instancedDrawArgs.TransformBuffer);
    RenderCommand::DrawIndexedInstanced(submitArgs.TargetVertexArray, instancedDrawArgs.NumInstances);
}

void Renderer::InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader)
{
    Debug::InitializeDebugDraw(debugShader);
}

void Renderer::UploadUniforms(const std::shared_ptr<Shader>& shader, const glm::mat4& transform, uint32_t cubeMapTextureUnit)
{
    shader->SetUniform("u_projection_view", s_RendererData.ProjectionViewMatrix);
    shader->SetUniform("u_transform", transform);
    shader->SetUniform("u_view", s_RendererData.ViewMatrix);
    shader->SetUniform("u_camera_location", s_RendererData.CameraPosition);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader->SetUniform("u_normal_transform", normalMatrix);
    s_LightBuffer->BindBuffer(*shader, "Lights");
    shader->SetUniform("u_num_lights", s_LightBuffer->ActualNumLights);

    Skybox::s_Instance->GetCubeMap()->Bind(cubeMapTextureUnit);
    shader->SetSamplerUniform("u_skybox_texture", Skybox::s_Instance->GetCubeMap(), cubeMapTextureUnit);
}