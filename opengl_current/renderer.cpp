#include "renderer.h"
#include "error_macros.h"
#include "render_command.h"
#include "debug_render_batch.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <array>

#include <GL/glew.h>

glm::mat4 Renderer::view_{1.0f};
glm::mat4 Renderer::projection_{1.0f};
glm::mat4 Renderer::view_projection_{1.0f};
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

    int colors_width = 4;
    int color_height = 4;

    default_texture_ = std::make_shared<Texture2D>(colors, TextureSpecification{colors_width, color_height, TextureFormat::kRgb});
    RenderCommand::Initialize();

    debug_batch_ = new DebugRenderBatch();
    RenderCommand::ClearBufferBindings_Debug();
    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(const CameraProjection& projection)
{
    projection_ = glm::perspective(glm::radians(projection.fov), projection.aspect_ratio, projection.z_near, projection.z_far);
}

void Renderer::BeginScene(glm::vec3 camera_pos, glm::quat camera_rotation)
{
    RenderCommand::BeginScene();
    view_ = glm::inverse(glm::translate(glm::identity<glm::mat4>(), camera_pos) * glm::mat4_cast(camera_rotation));
    view_projection_ = projection_ * view_;
    camera_position_ = camera_pos;
}

void Renderer::EndScene()
{
    RenderCommand::SetLineWidth(1);
    RenderCommand::EndScene();
}

void Renderer::SubmitTriangles(const SubmitCommandArgs& submit_args)
{
    ASSERT(submit_args.num_indices <= submit_args.vertex_array->GetNumIndices());

    Shader& shader = submit_args.GetShader();
    shader.Use();
    submit_args.SetupShader();

    UploadUniforms(shader, submit_args.transform);
    RenderCommand::DrawTriangles(*submit_args.vertex_array, submit_args.num_indices);
}

void Renderer::SubmitLines(const SubmitCommandArgs& submit_args)
{
    ASSERT(submit_args.num_indices <= submit_args.vertex_array->GetNumIndices());

    Shader& shader = submit_args.GetShader();
    shader.Use();
    submit_args.SetupShader();

    UploadUniforms(shader, submit_args.transform);
    RenderCommand::DrawLines(*submit_args.vertex_array, submit_args.num_indices);
}

void Renderer::SubmitPoints(const SubmitCommandArgs& submit_args)
{
    ASSERT(submit_args.num_indices <= submit_args.vertex_array->GetNumIndices());

    Shader& shader = submit_args.GetShader();
    shader.Use();
    submit_args.SetupShader();

    UploadUniforms(shader, submit_args.transform);
    RenderCommand::DrawPoints(*submit_args.vertex_array, submit_args.num_indices);
}

void Renderer::SubmitSkeleton(const SubmitCommandArgs& submit_args, std::span<const glm::mat4> transforms)
{
    Shader& shader = submit_args.GetShader();

    shader.Use();
    submit_args.SetupShader();

    UploadUniforms(shader, submit_args.transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms);
    RenderCommand::DrawTriangles(*submit_args.vertex_array, submit_args.num_indices);
}

void Renderer::SubmitMeshInstanced(const SubmitCommandArgs& submit_args, const UniformBuffer& transform_buffer, int num_instances)
{
    Shader& shader = submit_args.GetShader();

    shader.Use();
    submit_args.SetupShader();
    shader.BindUniformBuffer(shader.GetUniformBlockIndex("Transforms"), transform_buffer);
    UploadUniforms(shader, submit_args.transform);

    RenderCommand::DrawTrianglesInstanced(*submit_args.vertex_array, num_instances);
}

void Renderer::DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform, const glm::vec4& color)
{
    debug_batch_->AddBoxInstance(boxmin, boxmax, transform, color);
}

void Renderer::FlushDrawDebug(Material& shader)
{
    debug_batch_->FlushDraw(shader);
}

bool Renderer::IsVisibleToCamera(glm::vec3 worldspace_position, glm::vec3 bbox_min, glm::vec3 bbox_max)
{
    // only need apply translation for testing
    glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), worldspace_position);

    // Apply transformations to the bounding box
    glm::vec3 min_box_ws = transform * glm::vec4(bbox_min, 1.0f);
    glm::vec3 max_box_ws = transform * glm::vec4(bbox_max, 1.0f);

    // Calculate the bounding box corners in clip space
    glm::vec4 min_clipspace = view_projection_ * glm::vec4(min_box_ws, 1.0f);
    glm::vec4 max_clipspace = view_projection_ * glm::vec4(max_box_ws, 1.0f);

    // Check for intersection with the view frustum
    return !(max_clipspace.x < -max_clipspace.w || min_clipspace.x > min_clipspace.w ||
        max_clipspace.y < -max_clipspace.w || min_clipspace.y > min_clipspace.w ||
        max_clipspace.z < -max_clipspace.w || min_clipspace.z > min_clipspace.w);
}

void Renderer::UploadUniforms(Shader& shader, const glm::mat4& transform)
{
    shader.SetUniform("u_projection_view", view_projection_);
    shader.SetUniform("u_transform", transform);
    shader.SetUniform("u_view", view_);
    shader.SetUniform("u_camera_location", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniform("u_normal_transform", normal_matrix);
}