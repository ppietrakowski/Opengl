#include "Renderer.hpp"
#include "ErrorMacros.hpp"
#include "RenderCommand.hpp"
#include "Skybox.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

RendererData Renderer::s_RendererData{};
std::shared_ptr<Texture2D> Renderer::s_DefaultTexture;

static constexpr RgbColor Black{0, 0, 0};
static constexpr RgbColor Magenta{255, 0, 255};

class LightBuffer
{
public:
    LightBuffer(int numLights) :
        m_UniformBuffer(static_cast<int>(numLights * sizeof(LightData)))
    {
    }

public:

    void AddLight(const LightData& lightData)
    {
        m_UniformBuffer.UpdateElement(lightData, m_ActualNumLights);
        m_ActualNumLights++;
    }

    void Clear()
    {
        m_ActualNumLights = 0;
    }

    void BindBuffer(Shader& shader, const std::string& bindingName) const
    {
        shader.BindUniformBuffer(shader.GetUniformBlockIndex(bindingName), m_UniformBuffer);
    }

    int GetNumLights() const
    {
        return m_ActualNumLights;
    }

private:
    UniformBuffer m_UniformBuffer;
    int m_ActualNumLights{0};
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

void Renderer::Submit(const StaticMeshEntry& meshEntry, const glm::mat4& transform)
{
    StartSubmiting(meshEntry.GetMaterial(), transform);
    RenderCommand::DrawIndexed(meshEntry.GetVertexArray(), meshEntry.GetNumIndices());
}

void Renderer::SubmitSkeleton(const SkeletalMesh& skeletalMesh, const glm::mat4& transform, std::span<const glm::mat4> boneTransforms)
{
    const Material& material = *skeletalMesh.MainMaterial;

    std::shared_ptr<Shader> shader = material.GetShader();
    StartSubmiting(material, transform);
    shader->SetUniformMat4Array("u_BoneTransforms", boneTransforms);

    const VertexArray& vertexArray = skeletalMesh.GetVertexArray();
    RenderCommand::DrawIndexed(vertexArray, vertexArray.GetNumIndices());
}

void Renderer::SubmitMeshInstanced(const StaticMeshEntry& mesh, const Material& material, const UniformBuffer& buffer, int numInstances, const glm::mat4& transform)
{
    std::shared_ptr<Shader> shader = material.GetShader();
    StartSubmiting(material, transform);

    shader->BindUniformBuffer(shader->GetUniformBlockIndex("Transforms"), buffer);

    RenderCommand::DrawIndexedInstanced(mesh.GetVertexArray(), numInstances);
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

    int colorsWidth = 4;
    int colorsHeight = 4;

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

void Renderer::StartSubmiting(const Material& material, const glm::mat4& transform)
{
    material.SetupRenderState();
    std::shared_ptr<Shader> shader = material.GetShader();
    shader->Use();

    uint32_t cubeMapTextureUnit = material.GetNumTextures();
    UploadUniforms(shader, transform, cubeMapTextureUnit);
    material.SetShaderUniforms();
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