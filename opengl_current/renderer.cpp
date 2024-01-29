#include "renderer.h"
#include "error_macros.h"
#include "render_command.h"
#include "skybox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

#include <GL/glew.h>

RendererData Renderer::s_RendererData{};
std::shared_ptr<Texture2D> Renderer::s_DefaultTexture;

static constexpr RgbColor Black{0, 0, 0};
static constexpr RgbColor Magenta{255, 0, 255};

class LightBuffer
{
public:
    LightBuffer(int numLights) :
        m_UniformBuffer(std::make_shared<UniformBuffer>(static_cast<int32_t>(numLights * sizeof(LightData))))
    {
    }

public:

    void AddLight(const LightData& lightData)
    {
        m_UniformBuffer->UpdateBuffer(&lightData, sizeof(LightData), m_ActualNumLights * sizeof(LightData));
        m_ActualNumLights++;
    }

    void Clear()
    {
        m_ActualNumLights = 0;
    }

    void BindBuffer(Shader& shader, const std::string& bindingName) const
    {
        shader.BindUniformBuffer(shader.GetUniformBlockIndex(bindingName), *m_UniformBuffer);
    }

    int32_t GetNumLights() const
    {
        return m_ActualNumLights;
    }

private:
    std::shared_ptr<UniformBuffer> m_UniformBuffer;
    int32_t m_ActualNumLights{0};
};

static LightBuffer* s_LightBuffer = nullptr;

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    s_RendererData.Projection = projection;
    s_RendererData.ProjectionMatrix = glm::perspective(glm::radians(projection.Fov), projection.AspectRatio, projection.ZNear, projection.ZFar);
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
}

void Renderer::EndScene()
{
    RenderCommand::SetLineWidth(1);
    s_LightBuffer->Clear();
    RenderCommand::EndScene();
}

void Renderer::Submit(const SubmitCommandArgs& submitArgs)
{
    StartSubmiting(submitArgs);
    RenderCommand::DrawIndexed(submitArgs.GetVertexArray(), submitArgs.GetNumIndices());
}

void Renderer::SubmitSkeleton(const SubmitCommandArgs& submitArgs, std::span<const glm::mat4> transforms)
{
    std::shared_ptr<Shader> shader = submitArgs.GetShader();
    StartSubmiting(submitArgs);
    shader->SetUniformMat4Array("u_BoneTransforms", transforms);

    RenderCommand::DrawIndexed(submitArgs.GetVertexArray(), submitArgs.GetNumIndices());
}

void Renderer::SubmitMeshInstanced(const InstancedDrawArgs& instancedDrawArgs)
{
    const SubmitCommandArgs& submitArgs = instancedDrawArgs.GetSubmitArgs();
    std::shared_ptr<Shader> shader = submitArgs.GetShader();
    StartSubmiting(submitArgs);

    instancedDrawArgs.UploadTransform(*shader);
    RenderCommand::DrawIndexedInstanced(submitArgs.GetVertexArray(), instancedDrawArgs.GetNumInstances());
}

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

    int32_t colorsWidth = 4;
    int32_t colorsHeight = 4;

    s_DefaultTexture = std::make_shared<Texture2D>(colors, TextureSpecification{colorsWidth, colorsHeight, TextureFormat::Rgb});
    s_DefaultTexture->SetFilteringType(FilteringType::Nearest);
    RenderCommand::Initialize();

    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);

    s_LightBuffer = new LightBuffer(32);
}

void Renderer::Quit()
{
    SafeDelete(s_LightBuffer);

    s_DefaultTexture.reset();
    RenderCommand::Quit();
}

void Renderer::StartSubmiting(const SubmitCommandArgs& submitArgs)
{
    submitArgs.SetupRenderState();
    std::shared_ptr<Shader> shader = submitArgs.GetShader();
    shader->Use();

    uint32_t cubeMapTextureUnit = submitArgs.GetNumTexturesUsed();
    UploadUniforms(shader, submitArgs.GetTransform(), cubeMapTextureUnit);
    submitArgs.ApplyMaterialUniforms();
}

void Renderer::UploadUniforms(const std::shared_ptr<Shader>& shader, const glm::mat4& transform, uint32_t cubeMapTextureUnit)
{
    shader->SetUniform("u_ProjectionView", s_RendererData.ProjectionViewMatrix);
    shader->SetUniform("u_Transform", transform);
    shader->SetUniform("u_View", s_RendererData.ViewMatrix);
    shader->SetUniform("u_CameraLocation", s_RendererData.CameraPosition);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader->SetUniform("u_NormalTransform", normalMatrix);
    s_LightBuffer->BindBuffer(*shader, "Lights");
    shader->SetUniform("u_NumLights", s_LightBuffer->GetNumLights());

    std::shared_ptr<CubeMap> cubeMap = Skybox::s_Instance->GetCubeMap();
    cubeMap->Bind(cubeMapTextureUnit);
    shader->SetSamplerUniform("u_SkyboxTexture", Skybox::s_Instance->GetCubeMap(), cubeMapTextureUnit);
}