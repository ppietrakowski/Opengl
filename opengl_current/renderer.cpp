#include "renderer.h"
#include "buffer.h"
#include "error_macros.h"
#include "render_command.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <array>

glm::mat4 Renderer::view_{ 1.0f };
glm::mat4 Renderer::projection_{ 1.0f };
glm::mat4 Renderer::projection_view_{ 1.0f };
glm::vec3 Renderer::camera_position_{ 0.0f, 0.0f, 0.0f };
std::shared_ptr<Texture2D> Renderer::default_texture_;

// Predefined box indices (base for offsets for box batching)
static const uint32_t kBaseBoxIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr uint32_t kMaxDebugNumBox = 100;
constexpr uint32_t kNumBoxVertices = 8;
constexpr uint32_t kMaxIndices = kMaxDebugNumBox * ARRAY_NUM_ELEMENTS(kBaseBoxIndices);

/* Just does bind and unbind within scope */
struct DebugVertexArrayScope {

    VertexArray* target;

    DebugVertexArrayScope(VertexArray& target) :
        target{ &target } {
        target.Bind();
    }

    ~DebugVertexArrayScope() {
        target->Unbind();
    }
};

struct DebugRenderBatch {
    Buffer<glm::vec3> vertices;
    Buffer<uint32_t> indices;
    VertexArray vertex_array;
    uint32_t last_index_number{ 0 };

    DebugRenderBatch() :
        vertices{ kMaxDebugNumBox * kNumBoxVertices },
        indices{ kMaxIndices } {
        // initialize box batching
        VertexAttribute attributes[] = { {3, PrimitiveVertexType::kFloat} };

        vertex_array.AddDynamicBuffer(vertices.GetCapacityBytes(), attributes);
        IndexBuffer index_buffer(indices.GetCapacity());
        vertex_array.SetIndexBuffer(std::move(index_buffer));

        vertex_array.Unbind();
    }

    void UploadBatchedData() {
        DebugVertexArrayScope bind_array_scope{ vertex_array };

        VertexBuffer& vertex_buffer = vertex_array.GetVertexBufferAt(0);
        vertex_buffer.UpdateVertices(vertices.GetRawData(), vertices.GetSizeBytes());

        IndexBuffer& index_buffer = vertex_array.GetIndexBuffer();
        index_buffer.UpdateIndices(indices.GetRawData(), indices.GetSize());
    }

    void FlushDraw(Shader& shader) {
        Renderer::Submit(shader, indices.GetSize(), vertex_array, glm::mat4{ 1.0 }, RenderPrimitive::kLines);
        vertices.ResetPtrToStart();
        indices.ResetPtrToStart();
        last_index_number = 0;

        vertex_array.Unbind();
    }

    void AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform) {
        if (!CanBatchAnotherMesh(ARRAY_NUM_ELEMENTS(kBaseBoxIndices))) {
            return;
        }

        std::array<glm::vec3, 8> box_vertices = {
            glm::vec3{boxmin[0], boxmin[1], boxmin[2]},
            glm::vec3{boxmax[0], boxmin[1], boxmin[2]},
            glm::vec3{boxmax[0], boxmax[1], boxmin[2]},
            glm::vec3{boxmin[0], boxmax[1], boxmin[2]},

            glm::vec3{boxmin[0], boxmin[1], boxmax[2]},
            glm::vec3{boxmax[0], boxmin[1], boxmax[2]},
            glm::vec3{boxmax[0], boxmax[1], boxmax[2]},
            glm::vec3{boxmin[0], boxmax[1], boxmax[2]}
        };

        uint32_t max_num_vertices = static_cast<uint32_t>(box_vertices.size());

        for (uint32_t i = 0; i < max_num_vertices; ++i) {
            vertices.AddInstance(transform * glm::vec4{ box_vertices[i], 1.0f });
        }

        for (uint32_t i = 0; i < ARRAY_NUM_ELEMENTS(kBaseBoxIndices); ++i) {
            indices.AddInstance(kBaseBoxIndices[i] + last_index_number);
        }

        last_index_number += ARRAY_NUM_ELEMENTS(box_vertices);
    }

    bool CanBatchAnotherMesh(uint32_t num_indices) const {
        return last_index_number + num_indices < kMaxIndices && vertices.GetSize() < vertices.GetCapacity();
    }
};

static DebugRenderBatch* box_batch_ = nullptr;

void Renderer::Quit() {
    delete box_batch_;
    default_texture_.reset();
}

struct RgbColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(uint8_t red, uint8_t green, uint8_t blue) :
        red{ red },
        green{ green },
        blue{ blue } {}
};

static constexpr RgbColor kBlack{ 0, 0, 0 };
static constexpr RgbColor kMagenta{ 255, 0, 255 };

void Renderer::Initialize() {
    // array of checkerboard with black and magenta
    RgbColor colors[4][4] =
    {
        {kBlack, kBlack, kMagenta, kMagenta},
        {kBlack, kBlack, kMagenta, kMagenta},
        {kMagenta, kMagenta, kBlack, kBlack},
        {kMagenta, kMagenta, kBlack, kBlack}
    };

    uint32_t colors_width = 4;
    uint32_t colors_height = 4;

    default_texture_ = std::make_shared<Texture2D>(colors, colors_width, colors_height, TextureFormat::kRgb);

    glEnable(GL_DEPTH_TEST);

    box_batch_ = new DebugRenderBatch();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(float width, float height, float fov, float z_near, float z_far) {
    float aspect_ratio = width / height;
    projection_ = glm::perspective(glm::radians(fov), aspect_ratio, z_near, z_far);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 camera_position) {
    RenderCommand::BeginScene();
    view_ = view;
    projection_view_ = projection_ * view;
    camera_position_ = camera_position;
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
    RenderCommand::DrawIndexed(vertex_array, vertex_array.GetNumIndices(), render_primitive);
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, uint32_t count, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    Shader& shader = material.GetShader();

    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    UploadUniforms(shader, transform);
    shader.SetUniformMat4Array("u_bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(vertex_array, vertex_array.GetNumIndices(), render_primitive);
}


void Renderer::Submit(Shader& shader,
    const VertexArray& vertex_array,
    const glm::mat4& transform,
    RenderPrimitive render_primitive) {
    Submit(shader, vertex_array.GetNumIndices(), vertex_array, transform, render_primitive);
}

void Renderer::Submit(Shader& shader, uint32_t numIndices, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    ASSERT(numIndices <= vertex_array.GetNumIndices());

    shader.Use();
    UploadUniforms(shader, transform);
    RenderCommand::DrawIndexed(vertex_array, numIndices, render_primitive);
}

void Renderer::AddDebugBox(glm::vec3 box_min, glm::vec3 box_max, const glm::mat4& transform) {
    if (box_batch_->vertices.GetSize() == 0) {
        RenderCommand::SetLineWidth(2);
    }

    box_batch_->AddBoxInstance(box_min, box_max, transform);
}

void Renderer::FlushDrawDebug(Shader& shader) {
    box_batch_->UploadBatchedData();
    box_batch_->FlushDraw(shader);
}

void Renderer::UploadUniforms(Shader& shader, const glm::mat4& transform) {
    shader.SetUniformMat4("u_projection_view", projection_view_);
    shader.SetUniformMat4("u_transform", transform);
    shader.SetUniformMat4("u_view", view_);
    shader.SetUniformVec3("u_camera_location", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_normal_transform", normal_matrix);
}