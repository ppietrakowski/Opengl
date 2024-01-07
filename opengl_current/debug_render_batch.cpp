#include "debug_render_batch.h"
#include "renderer.h"

#include <array>

// Predefined box indices (base for offsets for box batching)
static const std::array<std::uint32_t, 24> kBaseBoxIndices =
{
    0u, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr std::int32_t kMaxDebugNumBox = 100;
constexpr std::int32_t kNumBoxVertices = 8;
constexpr std::int32_t kMaxIndices = kMaxDebugNumBox * STD_ARRAY_NUM_ELEMENTS(kBaseBoxIndices);

/* Just does bind and unbind within scope */
struct DebugVertexArrayScope
{
    VertexArray* target;

    DebugVertexArrayScope(VertexArray& target) :
        target{&target}
    {
        target.Bind();
    }

    ~DebugVertexArrayScope()
    {
        target->Unbind();
    }
};

DebugRenderBatch::DebugRenderBatch() :
    vertices_{kMaxDebugNumBox * kNumBoxVertices},
    indices_{kMaxIndices},
    vertex_array_{VertexArray::Create()}
{
    // initialize box batching
    VertexAttribute attributes[] = {{3, PrimitiveVertexType::kFloat}};

    vertex_array_->AddDynamicBuffer(vertices_.GetCapacityBytes(), attributes);
    vertex_array_->SetIndexBuffer(IndexBuffer::CreateEmpty(indices_.GetCapacity()));

    vertex_array_->Unbind();
}

void DebugRenderBatch::UploadBatchedData()
{
    DebugVertexArrayScope bind_array_scope{*vertex_array_};

    std::shared_ptr<VertexBuffer> vertex_buffer = vertex_array_->GetVertexBufferAt(0);
    vertex_buffer->UpdateVertices(vertices_.GetRawData(), vertices_.GetSizeBytes());

    std::shared_ptr<IndexBuffer> index_buffer = vertex_array_->GetIndexBuffer();
    index_buffer->UpdateIndices(indices_.GetRawData(), indices_.GetSize());
}

void DebugRenderBatch::FlushDraw(Shader& shader)
{
    Renderer::Submit(shader, indices_.GetSize(), *vertex_array_, glm::mat4{1.0}, RenderPrimitive::kLines);
    vertices_.ResetPtrToStart();
    indices_.ResetPtrToStart();
    last_index_number_ = 0;
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform)
{
    if (!CanBatchAnotherMesh(ARRAY_NUM_ELEMENTS(kBaseBoxIndices)))
    {
        return;
    }

    std::array<glm::vec3, 8> boxVertices = {
        glm::vec3{boxmin[0], boxmin[1], boxmin[2]},
        glm::vec3{boxmax[0], boxmin[1], boxmin[2]},
        glm::vec3{boxmax[0], boxmax[1], boxmin[2]},
        glm::vec3{boxmin[0], boxmax[1], boxmin[2]},

        glm::vec3{boxmin[0], boxmin[1], boxmax[2]},
        glm::vec3{boxmax[0], boxmin[1], boxmax[2]},
        glm::vec3{boxmax[0], boxmax[1], boxmax[2]},
        glm::vec3{boxmin[0], boxmax[1], boxmax[2]}
    };


    for (const glm::vec3& vertex : boxVertices)
    {
        vertices_.AddInstance(transform * glm::vec4{vertex, 1.0f});
    }

    for (std::uint32_t index : kBaseBoxIndices)
    {
        indices_.AddInstance(index + last_index_number_);
    }

    std::int32_t max_num_indices = STD_ARRAY_NUM_ELEMENTS(boxVertices);
    last_index_number_ += max_num_indices;
}

bool DebugRenderBatch::CanBatchAnotherMesh(std::int32_t num_indices) const
{
    return last_index_number_ + num_indices < kMaxIndices && vertices_.GetSize() < vertices_.GetCapacity();
}

bool DebugRenderBatch::HasBatchedAnyPrimitive() const
{
    return vertices_.GetSize() > 0;
}
