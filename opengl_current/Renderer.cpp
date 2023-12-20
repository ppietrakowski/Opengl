#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>


#include "ErrorMacros.h"
#include "RenderCommand.h"

#include <array>

glm::mat4 Renderer::view_{ 1.0f };
glm::mat4 Renderer::projection_{ 1.0f };
glm::mat4 Renderer::projection_view_{ 1.0f };
glm::vec3 Renderer::camera_position_{ 0.0f, 0.0f, 0.0f };
std::shared_ptr<Texture2D> Renderer::default_texture_;

// Predefined box indices (base for offsets for IndexBuffer)
static const std::uint32_t kBaseBoxIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr std::size_t kMaxDebugNumBox = 100;
constexpr std::size_t kNumBoxVertices = 8;

struct BoxBatchData {
    glm::vec3 vertices[kMaxDebugNumBox * kNumBoxVertices];
    std::uint32_t max_num_boxes{ kMaxDebugNumBox };
    std::uint32_t current_num_box{ 0 };
    VertexArray vertex_array;

    BoxBatchData() {
        // initialize box batching
        VertexAttribute attributes[] = { {3, PrimitiveVertexType::kFloat} };
        VertexBuffer buffer{};

        vertex_array.AddDynamicBuffer(static_cast<std::uint32_t>(kNumBoxVertices * kMaxDebugNumBox * sizeof(glm::vec3)), attributes);

        // prebatch indices
        std::vector<std::uint32_t> indices;
        std::uint32_t indices_start_offset = 0;
        std::uint32_t max_num_indices = static_cast<std::uint32_t>(ARRAY_NUM_ELEMENTS(kBaseBoxIndices));

        indices.reserve(kMaxDebugNumBox * max_num_indices);

        // current starting index of mesh index
        std::uint32_t offset_to_next_free_index = kNumBoxVertices;

        for (std::uint32_t i = 0; i < kMaxDebugNumBox; i++) {
            for (std::uint32_t j = 0; j < max_num_indices; j++) {
                std::uint32_t vertexIndex = kBaseBoxIndices[j] + indices_start_offset;
                indices.emplace_back(vertexIndex);
            }

            indices_start_offset += offset_to_next_free_index;
        }


        IndexBuffer indexBuffer(indices.data(), static_cast<std::uint32_t>(indices.size()));
        vertex_array.SetIndexBuffer(std::move(indexBuffer));
    }

    void UpdateBuffers() {
        // only the vertex buffer should be updated, because indices are prebatched 
        VertexBuffer& vertexBuffer = vertex_array.GetVertexBufferAt(0);
        vertexBuffer.UpdateVertices(vertices, sizeof(glm::vec3) * current_num_box * kNumBoxVertices);
    }

    void FlushDraw(Shader& shader) {
        Renderer::Submit(shader, current_num_box * ARRAY_NUM_ELEMENTS(kBaseBoxIndices), vertex_array, glm::mat4{ 1.0 }, RenderPrimitive::kLines);
        current_num_box = 0;
    }

    void AddInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform) {
        std::array<glm::vec3, 8> vertices = {
            glm::vec3{boxmin[0], boxmin[1], boxmin[2]},
            glm::vec3{boxmax[0], boxmin[1], boxmin[2]},
            glm::vec3{boxmax[0], boxmax[1], boxmin[2]},
            glm::vec3{boxmin[0], boxmax[1], boxmin[2]},

            glm::vec3{boxmin[0], boxmin[1], boxmax[2]},
            glm::vec3{boxmax[0], boxmin[1], boxmax[2]},
            glm::vec3{boxmax[0], boxmax[1], boxmax[2]},
            glm::vec3{boxmin[0], boxmax[1], boxmax[2]}
        };

        if (current_num_box >= max_num_boxes) {
            return;
        }

        std::uint32_t max_num_vertices = static_cast<std::uint32_t>(vertices.size());

        for (std::uint32_t i = 0; i < max_num_vertices; ++i) {
            glm::vec3& vertex = this->vertices[i + current_num_box * kNumBoxVertices];
            vertex = transform * glm::vec4{ vertices[i], 1.0f };
        }

        current_num_box++;
    }
};

static BoxBatchData* box_batch_ = nullptr;

void Renderer::Quit() {
    delete box_batch_;
    default_texture_.reset();
}

struct RgbColor {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) :
        red{ red },
        green{ green },
        blue{ blue } {}
};

static constexpr RgbColor kBlack{ 0, 0, 0 };
static constexpr RgbColor kMagenta{ 255, 0, 255 };

void Renderer::Initialize() {
    // array of checkerboard with black and magenta
    RgbColor colors[4 * 4] =
    {
        kBlack, kBlack, kMagenta, kMagenta,
        kBlack, kBlack, kMagenta, kMagenta,
        kMagenta, kMagenta, kBlack, kBlack,
        kMagenta, kMagenta, kBlack, kBlack
    };

    default_texture_ = std::make_shared<Texture2D>(colors, 4, 4, TextureFormat::kRgb);

    glEnable(GL_DEPTH_TEST);

    box_batch_ = new BoxBatchData();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(float width, float height, float fov, float z_near, float z_far) {
    float aspectRatio = width / height;
    projection_ = glm::perspective(glm::radians(fov), aspectRatio, z_near, z_far);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 camera_position) {
    view_ = view;
    projection_view_ = projection_ * view;
    camera_position_ = camera_position;
}

void Renderer::EndScene() {
    RenderCommand::SetLineWidth(1);
}

void Renderer::Submit(const Material& material, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    shader.SetUniformMat4("u_ProjectionView", projection_view_);
    shader.SetUniformMat4("u_Transform", transform);
    shader.SetUniformVec3("u_CameraLocation", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_NormalTransform", normal_matrix);

    RenderCommand::DrawIndexed(vertex_array, vertex_array.GetNumIndices(), render_primitive);
}

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, std::uint32_t count, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    shader.SetUniformMat4("u_ProjectionView", projection_view_);
    shader.SetUniformMat4("u_Transform", transform);
    shader.SetUniformVec3("u_CameraLocation", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_NormalTransform", normal_matrix);
    shader.SetUniformMat4Array("bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(vertex_array, vertex_array.GetNumIndices(), render_primitive);
}

void Renderer::Submit(Shader& shader,
    const VertexArray& vertex_array,
    const glm::mat4& transform,
    RenderPrimitive render_primitive) {
    Submit(shader, vertex_array.GetNumIndices(), vertex_array, transform, render_primitive);
}

void Renderer::Submit(Shader& shader, std::uint32_t numIndices, const VertexArray& vertex_array, const glm::mat4& transform, RenderPrimitive render_primitive) {
    ASSERT(numIndices <= vertex_array.GetNumIndices());

    shader.Use();
    shader.SetUniformMat4("u_ProjectionView", projection_view_);
    shader.SetUniformMat4("u_Transform", transform);
    shader.SetUniformVec3("u_CameraLocation", camera_position_);

    glm::mat3 normal_matrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_NormalTransform", normal_matrix);

    RenderCommand::DrawIndexed(vertex_array, numIndices, render_primitive);
}

void Renderer::AddDebugBox(glm::vec3 box_min, glm::vec3 box_max, const glm::mat4& transform) {
    if (box_batch_->current_num_box == 0) {
        RenderCommand::SetLineWidth(2);
    }

    box_batch_->AddInstance(box_min, box_max, transform);
}

void Renderer::FlushDrawDebug(Shader& shader) {
    box_batch_->UpdateBuffers();
    box_batch_->FlushDraw(shader);
}
