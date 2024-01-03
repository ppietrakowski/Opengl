#include "renderer.h"
#include "buffer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <array>

glm::mat4 Renderer::View{1.0f};
glm::mat4 Renderer::Projection{1.0f};
glm::mat4 Renderer::ProjectionView{1.0f};
glm::vec3 Renderer::CameraPosition{0.0f, 0.0f, 0.0f};
std::shared_ptr<ITexture2D> Renderer::DefaultTexture;

static DebugRenderBatch* s_DebugBatch = nullptr;

void Renderer::Quit()
{
    delete s_DebugBatch;
    DefaultTexture.reset();
    RenderCommand::Quit();
}

struct RgbColor
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(uint8_t red, uint8_t green, uint8_t blue) :
        Red{red},
        Green{green},
        Blue{blue}
    {
    }
};

static constexpr RgbColor kBlack{0, 0, 0};
static constexpr RgbColor kMagenta{255, 0, 255};

void Renderer::Initialize()
{
    // array of checkerboard with black and magenta
    RgbColor colors[4][4] =
    {
        {kBlack, kBlack, kMagenta, kMagenta},
        {kBlack, kBlack, kMagenta, kMagenta},
        {kMagenta, kMagenta, kBlack, kBlack},
        {kMagenta, kMagenta, kBlack, kBlack}
    };

    int32_t colorsWidth = 4;
    int32_t colorsHeight = 4;

    DefaultTexture = ITexture2D::Create(colors, TextureSpecification{colorsWidth, colorsHeight, TextureFormat::kRgb});
    RenderCommand::Initialize();

    s_DebugBatch = new DebugRenderBatch();
    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    Projection = glm::perspective(glm::radians(projection.Fov), projection.AspectRatio, projection.ZNear, projection.ZFar);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 cameraPosition)
{
    RenderCommand::BeginScene();
    View = view;
    ProjectionView = Projection * view;
    CameraPosition = cameraPosition;
}

void Renderer::EndScene()
{
    RenderCommand::SetLineWidth(1);
    RenderCommand::EndScene();
}

void Renderer::Submit(const Material& material, const IVertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    IShader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, vertexArray.GetNumIndices(), renderPrimitive});
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, int32_t count, const IVertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    IShader& shader = material.GetShader();

    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, vertexArray.GetNumIndices(), renderPrimitive});
}


void Renderer::Submit(IShader& shader,
    const IVertexArray& vertexArray,
    const glm::mat4& transform,
    RenderPrimitive renderPrimitive)
{
    Submit(shader, vertexArray.GetNumIndices(), vertexArray, transform, renderPrimitive);
}

void Renderer::Submit(IShader& shader, int32_t numIndices, const IVertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    ASSERT(numIndices <= vertexArray.GetNumIndices());

    shader.Use();
    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertexArray, vertexArray.GetNumIndices(), renderPrimitive});
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform)
{
    if (!s_DebugBatch->HasBatchedAnyPrimitive())
    {
        RenderCommand::SetLineWidth(2);
    }

    s_DebugBatch->AddBoxInstance(boxmin, boxmax, transform);
}

void Renderer::FlushDrawDebug(IShader& shader)
{
    s_DebugBatch->UploadBatchedData();
    s_DebugBatch->FlushDraw(shader);
}

void Renderer::UploadUniforms(IShader& shader, const glm::mat4& transform)
{
    shader.SetUniformMat4("u_projection_view", ProjectionView);
    shader.SetUniformMat4("u_transform", transform);
    shader.SetUniformMat4("u_view", View);
    shader.SetUniformVec3("u_camera_location", CameraPosition);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_normal_transform", normalMatrix);
}