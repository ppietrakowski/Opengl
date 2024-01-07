#include "renderer.h"
#include "buffer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <array>

glm::mat4 Renderer::view_{1.0f};
glm::mat4 Renderer::projection_{1.0f};
glm::mat4 Renderer::projection_view_{1.0f};
glm::vec3 Renderer::camera_position_{0.0f, 0.0f, 0.0f};
std::shared_ptr<Texture2D> Renderer::default_texture_;

static DebugRenderBatch* debug_batch_ = nullptr;

void Renderer::Quit()
{
    delete debug_batch_;
    default_texture_.reset();
    RenderCommand::Quit();
}

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

    std::int32_t colors_width = 4;
    std::int32_t colors_height = 4;

    default_texture_ = Texture2D::Create(colors, TextureSpecification{colors_width, colors_height, TextureFormat::kRgb});
    RenderCommand::Initialize();

    debug_batch_ = new DebugRenderBatch();
    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    projection_ = glm::perspective(glm::radians(projection.fov), projection.aspect_ratio, projection.z_near, projection.z_far);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 camera_pos)
{
    RenderCommand::BeginScene();
    view_ = view;
    projection_view_ = projection_ * view;
    camera_position_ = camera_pos;
}

void Renderer::EndScene()
{
    RenderCommand::SetLineWidth(1);
    RenderCommand::EndScene();
}

void Renderer::Submit(const Material& material, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive)
{
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertex_array, vertex_array.GetNumIndices(), render_primitive});
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms,
    std::int32_t count, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive)
{
    Shader& shader = material.GetShader();

    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertex_array, vertex_array.GetNumIndices(), render_primitive});
}


void Renderer::Submit(Shader& shader,
    const VertexArray& vertex_array,
    const glm::mat4& transform,
    RenderPrimitive render_primitive)
{
    Submit(shader, vertex_array.GetNumIndices(), vertex_array, transform, render_primitive);
}

void Renderer::Submit(Shader& shader, std::int32_t num_indices, const VertexArray& vertex_array,
    const glm::mat4& transform, RenderPrimitive render_primitive)
{
    ASSERT(num_indices <= vertex_array.GetNumIndices());

    shader.Use();
    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertex_array, vertex_array.GetNumIndices(), render_primitive});
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform)
{
    if (!debug_batch_->HasBatchedAnyPrimitive())
    {
        RenderCommand::SetLineWidth(2);
    }

    debug_batch_->AddBoxInstance(boxmin, boxmax, transform);
}

void Renderer::FlushDrawDebug(Shader& shader)
{
    debug_batch_->UploadBatchedData();
    debug_batch_->FlushDraw(shader);
}

void Renderer::UploadUniforms(Shader& shader, const glm::mat4& transform)
{
    shader.SetUniformMat4("u_projection_view", projection_view_);
    shader.SetUniformMat4("u_transform", transform);
    shader.SetUniformMat4("u_view", view_);
    shader.SetUniformVec3("u_camera_location", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_normal_transform", normal_matrix);
}