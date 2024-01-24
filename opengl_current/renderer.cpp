#include "renderer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"
#include "renderer_2d.h"
#include "skybox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

#include <GL/glew.h>

glm::mat4 Renderer::s_View{1.0f};
glm::mat4 Renderer::s_Projection{1.0f};
glm::mat4 Renderer::s_ViewProjection{1.0f};
glm::vec3 Renderer::s_CameraPosition{0.0f, 0.0f, 0.0f};
std::shared_ptr<Texture2D> Renderer::s_DefaultTexture;

static DebugRenderBatch* s_DebugBatch = nullptr;

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
    SafeDelete(s_DebugBatch);

    s_DefaultTexture.reset();
    Renderer2D::Quit();
    RenderCommand::Quit();
}

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    s_Projection = glm::perspective(glm::radians(projection.Fov), projection.AspectRatio, projection.ZNear, projection.ZFar);
    Renderer2D::UpdateProjection(projection);
}

void Renderer::BeginScene(glm::vec3 cameraPosition, glm::quat cameraRotation)
{
    RenderCommand::BeginScene();
    s_View = glm::inverse(glm::translate(glm::identity<glm::mat4>(), cameraPosition) * glm::mat4_cast(cameraRotation));
    s_ViewProjection = s_Projection * s_View;
    s_CameraPosition = cameraPosition;
}

void Renderer::EndScene()
{
    RenderCommand::SetLineWidth(1);

    if (s_DebugBatch)
    {
        Renderer::FlushDrawDebug();
    }

    Renderer2D::FlushDraw();
    s_LightBuffer->Clear();
    RenderCommand::EndScene();
}

void Renderer::SubmitTriangles(const SubmitCommandArgs& submitArgs)
{
    ASSERT(submitArgs.NumIndices <= submitArgs.TargetVertexArray->GetNumIndices());

    Shader& shader = submitArgs.GetShader();
    shader.Use();
    submitArgs.SetupShader();

    UploadUniforms(shader, submitArgs.Transform);

    uint32_t textureUnit = submitArgs.UsedMaterial->GetNumTextures();

    Skybox::s_Instance->GetCubeMap()->Bind(textureUnit);
    shader.SetSamplerUniform("u_skybox_texture", Skybox::s_Instance->GetCubeMap(), textureUnit);

    RenderCommand::DrawTriangles(*submitArgs.TargetVertexArray, submitArgs.NumIndices);
}

void Renderer::SubmitLines(const SubmitCommandArgs& submitArgs)
{
    ASSERT(submitArgs.NumIndices <= submitArgs.TargetVertexArray->GetNumIndices());

    Shader& shader = submitArgs.GetShader();
    shader.Use();
    submitArgs.SetupShader();

    UploadUniforms(shader, submitArgs.Transform);
    RenderCommand::DrawLines(*submitArgs.TargetVertexArray, submitArgs.NumIndices);
}

void Renderer::SubmitPoints(const SubmitCommandArgs& submitArgs)
{
    ASSERT(submitArgs.NumIndices <= submitArgs.TargetVertexArray->GetNumIndices());

    Shader& shader = submitArgs.GetShader();
    shader.Use();
    submitArgs.SetupShader();

    UploadUniforms(shader, submitArgs.Transform);
    RenderCommand::DrawPoints(*submitArgs.TargetVertexArray, submitArgs.NumIndices);
}

void Renderer::SubmitSkeleton(const SubmitCommandArgs& submitArgs, std::span<const glm::mat4> transforms)
{
    Shader& shader = submitArgs.GetShader();

    shader.Use();
    submitArgs.SetupShader();

    UploadUniforms(shader, submitArgs.Transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms);
    uint32_t textureUnit = submitArgs.UsedMaterial->GetNumTextures();
    Skybox::s_Instance->GetCubeMap()->Bind(textureUnit);
    shader.SetSamplerUniform("u_skybox_texture", Skybox::s_Instance->GetCubeMap(), textureUnit);

    RenderCommand::DrawTriangles(*submitArgs.TargetVertexArray, submitArgs.NumIndices);
}

void Renderer::SubmitMeshInstanced(const SubmitCommandArgs& submitArgs, const UniformBuffer& transformBuffer, int numInstances)
{
    Shader& shader = submitArgs.GetShader();

    submitArgs.UsedMaterial->SetupRenderState();
    shader.Use();
    submitArgs.UsedMaterial->SetShaderUniforms();

    shader.BindUniformBuffer(shader.GetUniformBlockIndex("Transforms"), transformBuffer);
    UploadUniforms(shader, submitArgs.Transform);

    uint32_t textureUnit = submitArgs.UsedMaterial->GetNumTextures();

    Skybox::s_Instance->GetCubeMap()->Bind(textureUnit);
    shader.SetSamplerUniform("u_skybox_texture", Skybox::s_Instance->GetCubeMap(), textureUnit);

    RenderCommand::DrawTrianglesInstanced(*submitArgs.TargetVertexArray, numInstances);
}

void Renderer::DrawDebugBox(const Box& box, const Transform& transform, const glm::vec4& Color)
{
    ASSERT(s_DebugBatch);
    s_DebugBatch->AddBoxInstance(box, transform, Color);
}

void Renderer::DrawDebugLine(const Line& line, const Transform& transform, const glm::vec4& Color)
{
    ASSERT(s_DebugBatch);
    s_DebugBatch->AddLineInstance(line, transform, Color);
}

void Renderer::FlushDrawDebug()
{
    ASSERT(s_DebugBatch);
    s_DebugBatch->FlushDraw();
}

bool Renderer::IsVisibleToCamera(glm::vec3 worldspacePosition, glm::vec3 bboxMin, glm::vec3 bboxMax)
{
    // only need apply translation for testing
    glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), worldspacePosition);

    // Apply transformations to the bounding box
    glm::vec3 minBoxWs = transform * glm::vec4(bboxMin, 1.0f);
    glm::vec3 maxBoxWs = transform * glm::vec4(bboxMax, 1.0f);

    // Calculate the bounding box corners in clip space
    glm::vec4 minClipspace = s_ViewProjection * glm::vec4(minBoxWs, 1.0f);
    glm::vec4 maxClipspace = s_ViewProjection * glm::vec4(maxBoxWs, 1.0f);

    // Check for intersection with the view frustum
    return !(maxClipspace.x < -maxClipspace.w || minClipspace.x > minClipspace.w ||
        maxClipspace.y < -maxClipspace.w || minClipspace.y > minClipspace.w ||
        maxClipspace.z < -maxClipspace.w || minClipspace.z > minClipspace.w);
}

void Renderer::AddLight(const LightData& lightData)
{
    s_LightBuffer->AddLight(lightData);
}

void Renderer::InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader)
{
    s_DebugBatch = new DebugRenderBatch(debugShader);
}

void Renderer::UploadUniforms(Shader& shader, const glm::mat4& transform)
{
    shader.SetUniform("u_projection_view", s_ViewProjection);
    shader.SetUniform("u_transform", transform);
    shader.SetUniform("u_view", s_View);
    shader.SetUniform("u_camera_location", s_CameraPosition);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader.SetUniform("u_normal_transform", normalMatrix);
    s_LightBuffer->BindBuffer(shader, "Lights");
    shader.SetUniform("u_num_lights", s_LightBuffer->ActualNumLights);
}