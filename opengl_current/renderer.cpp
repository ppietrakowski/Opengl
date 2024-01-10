#include "renderer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

glm::mat4 Renderer::s_View{1.0f};
glm::mat4 Renderer::s_Projection{1.0f};
glm::mat4 Renderer::s_ViewProjection{1.0f};
glm::vec3 Renderer::s_CameraPosition{0.0f, 0.0f, 0.0f};
std::shared_ptr<Texture2D> Renderer::s_DefaultTexture;

static DebugRenderBatch* s_DebugBatch = nullptr;

void Renderer::Quit()
{
    delete s_DebugBatch;
    s_DefaultTexture.reset();
    RenderCommand::Quit();
}

static constexpr RgbColor Black{0, 0, 0};
static constexpr RgbColor Magenta{255, 0, 255};

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

    std::int32_t colorsWidth = 4;
    std::int32_t colorsHeight = 4;

    s_DefaultTexture = Texture2D::Create(colors, TextureSpecification{colorsWidth, colorsHeight, TextureFormat::Rgb});
    RenderCommand::Initialize();

    s_DebugBatch = new DebugRenderBatch();
    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    s_Projection = glm::perspective(glm::radians(projection.Fov), projection.AspectRatio, projection.ZNear, projection.ZFar);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 cameraPos, glm::quat cameraRotation)
{
    RenderCommand::BeginScene();
    s_View = view;
    s_ViewProjection = s_Projection * view;
    s_CameraPosition = cameraPos;
}

void Renderer::EndScene()
{
    RenderCommand::SetLineWidth(1);
    RenderCommand::EndScene();
}

void Renderer::Submit(const Material& material, const VertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, vertexArray.GetNumIndices(), renderPrimitive});
}

void Renderer::Submit(const Material& material, std::int32_t numIndices, const VertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, numIndices, renderPrimitive});
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms,
    std::int32_t count, const VertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    Shader& shader = material.GetShader();

    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, vertexArray.GetNumIndices(), renderPrimitive});
}


void Renderer::Submit(Shader& shader,
    const VertexArray& vertexArray,
    const glm::mat4& transform,
    RenderPrimitive renderPrimitive)
{
    Submit(shader, vertexArray.GetNumIndices(), vertexArray, transform, renderPrimitive);
}

void Renderer::Submit(Shader& shader, std::int32_t numIndices, const VertexArray& vertexArray,
    const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    ASSERT(numIndices <= vertexArray.GetNumIndices());

    shader.Use();
    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, vertexArray.GetNumIndices(), renderPrimitive});
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform)
{
    s_DebugBatch->AddBoxInstance(boxmin, boxmax, transform);
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform, const glm::vec4& color)
{
    s_DebugBatch->AddBoxInstance(boxmin, boxmax, transform, color);
}

void Renderer::FlushDrawDebug(Material& shader)
{
    s_DebugBatch->FlushDraw(shader);
}

bool Renderer::IsVisibleToCamera(glm::vec3 worldspacePosition, glm::vec3 bboxMin, glm::vec3 bboxMax)
{
    // only need apply translation for testing
    glm::mat4 transformMatrix = glm::translate(glm::identity<glm::mat4>(), worldspacePosition);

    // Apply transformations to the bounding box
    glm::vec3 minBoxWs = transformMatrix * glm::vec4(bboxMin, 1.0f);
    glm::vec3 maxBoxWs = transformMatrix * glm::vec4(bboxMax, 1.0f);

    // Calculate the bounding box corners in clip space
    glm::vec4 minClipspace = s_ViewProjection * glm::vec4(minBoxWs, 1.0f);
    glm::vec4 maxClipspace = s_ViewProjection * glm::vec4(maxBoxWs, 1.0f);

    // Check for intersection with the view frustum
    return !(maxClipspace.x < -maxClipspace.w || minClipspace.x > minClipspace.w ||
        maxClipspace.y < -maxClipspace.w || minClipspace.y > minClipspace.w ||
        maxClipspace.z < -maxClipspace.w || minClipspace.z > minClipspace.w);
}

void Renderer::UploadUniforms(Shader& shader, const glm::mat4& transform)
{
    shader.SetUniform("u_projection_view", s_ViewProjection);
    shader.SetUniform("u_transform", transform);
    shader.SetUniform("u_view", s_View);
    shader.SetUniform("u_camera_location", s_CameraPosition);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader.SetUniform("u_normal_transform", normalMatrix);
}