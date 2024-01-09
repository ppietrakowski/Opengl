#include "renderer.h"
#include "buffer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

glm::mat4 Renderer::view_{1.0f};
glm::mat4 Renderer::projection_{1.0f};
glm::mat4 Renderer::projection_view_{1.0f};
glm::vec3 Renderer::camera_position_{0.0f, 0.0f, 0.0f};
std::shared_ptr<Texture2D> Renderer::default_texture_;

static DebugRenderBatch* debug_batch_ = nullptr;

void Renderer::Quit() {
    delete debug_batch_;
    default_texture_.reset();
    RenderCommand::Quit();
}

static constexpr RgbColor kBlack{0, 0, 0};
static constexpr RgbColor kMagenta{255, 0, 255};

void Renderer::Initialize() {
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

void Renderer::UpdateProjection(const CameraProjection& projection) {
    projection_ = glm::perspective(glm::radians(projection.fov), projection.aspect_ratio, projection.z_near, projection.z_far);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 camera_pos, glm::quat camera_rotation) {
    RenderCommand::BeginScene();
    view_ = view;
    projection_view_ = projection_ * view;
    camera_position_ = camera_pos;
}

void Renderer::EndScene() {
    RenderCommand::SetLineWidth(1);
    RenderCommand::EndScene();
}

void Renderer::Submit(const Material& material, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertex_array, vertex_array.GetNumIndices(), render_primitive});
}

void Renderer::Submit(const Material& material, std::int32_t num_indices, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertex_array, num_indices, render_primitive});
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms,
    std::int32_t count, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
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
    RenderPrimitive render_primitive) {
    Submit(shader, vertex_array.GetNumIndices(), vertex_array, transform, render_primitive);
}

void Renderer::Submit(Shader& shader, std::int32_t num_indices, const VertexArray& vertex_array,
    const glm::mat4& transform, RenderPrimitive render_primitive) {
    ASSERT(num_indices <= vertex_array.GetNumIndices());

    shader.Use();
    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(IndexedDrawData{&vertex_array, vertex_array.GetNumIndices(), render_primitive});
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform) {
    debug_batch_->AddBoxInstance(boxmin, boxmax, transform);
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform, const glm::vec4& color) {
    debug_batch_->AddBoxInstance(boxmin, boxmax, transform, color);
}

void Renderer::FlushDrawDebug(Material& shader) {
    debug_batch_->FlushDraw(shader);
}

bool Renderer::IsVisibleToCamera(glm::vec3 worldspace_position, glm::vec3 bbox_min, glm::vec3 bbox_max) {
    // only need apply translation for testing
    glm::mat4 transform_matrix = glm::translate(glm::identity<glm::mat4>(), worldspace_position);

    // Apply transformations to the bounding box
    glm::vec3 min_box_ws = transform_matrix * glm::vec4(bbox_min, 1.0f);
    glm::vec3 max_box_ws = transform_matrix * glm::vec4(bbox_max, 1.0f);

    // Calculate the bounding box corners in clip space
    glm::vec4 min_clipspace = projection_view_ * glm::vec4(min_box_ws, 1.0f);
    glm::vec4 max_clipspace = projection_view_ * glm::vec4(max_box_ws, 1.0f);

    // Check for intersection with the view frustum
    return !(max_clipspace.x < -max_clipspace.w || min_clipspace.x > min_clipspace.w ||
        max_clipspace.y < -max_clipspace.w || min_clipspace.y > min_clipspace.w ||
        max_clipspace.z < -max_clipspace.w || min_clipspace.z > min_clipspace.w);
}

void Renderer::UploadUniforms(Shader& shader, const glm::mat4& transform) {
    shader.SetUniform("u_projection_view", projection_view_);
    shader.SetUniform("u_transform", transform);
    shader.SetUniform("u_view", view_);
    shader.SetUniform("u_camera_location", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniform("u_normal_transform", normal_matrix);
}