#include "renderer.h"
#include "buffer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <array>

glm::mat4 Renderer::s_View{1.0f};
glm::mat4 Renderer::s_Projection{1.0f};
glm::mat4 Renderer::s_ProjectionView{1.0f};
glm::vec3 Renderer::s_CameraPosition{0.0f, 0.0f, 0.0f};
std::shared_ptr<ITexture2D> Renderer::s_DefaultTexture;

static DebugRenderBatch* s_DebugBatch = nullptr;

void Renderer::Quit()
{
    delete s_DebugBatch;
    s_DefaultTexture.reset();
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

    uint32_t colorsWidth = 4;
    uint32_t colorsHeight = 4;

    s_DefaultTexture = ITexture2D::Create(colors, colorsWidth, colorsHeight, TextureFormat::kRgb);
    RenderCommand::Initialize();

    s_DebugBatch = new DebugRenderBatch();
    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(float width, float height, float fov, float zNear, float zFar)
{
    float aspectRatio = width / height;
    s_Projection = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 cameraPosition)
{
    RenderCommand::BeginScene();
    s_View = view;
    s_ProjectionView = s_Projection * view;
    s_CameraPosition = cameraPosition;
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
    RenderCommand::DrawIndexed(vertexArray, vertexArray.GetNumIndices(), renderPrimitive);
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, uint32_t count, const IVertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    IShader& shader = material.GetShader();

    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(vertexArray, vertexArray.GetNumIndices(), renderPrimitive);
}


void Renderer::Submit(IShader& shader,
    const IVertexArray& vertexArray,
    const glm::mat4& transform,
    RenderPrimitive renderPrimitive)
{
    Submit(shader, vertexArray.GetNumIndices(), vertexArray, transform, renderPrimitive);
}

void Renderer::Submit(IShader& shader, uint32_t numIndices, const IVertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    ASSERT(numIndices <= vertexArray.GetNumIndices());

    shader.Use();
    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(vertexArray, numIndices, renderPrimitive);
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
    shader.SetUniformMat4("u_projection_view", s_ProjectionView);
    shader.SetUniformMat4("u_transform", transform);
    shader.SetUniformMat4("u_view", s_View);
    shader.SetUniformVec3("u_camera_location", s_CameraPosition);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_normal_transform", normalMatrix);
}