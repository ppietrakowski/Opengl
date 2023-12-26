#include "debug_render_batch.h"
#include "renderer.h"

#include <array>

// Predefined box indices (base for offsets for box batching)
static const std::array<uint32_t, 24> kBaseBoxIndices =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr uint32_t kMaxDebugNumBox = 100;
constexpr uint32_t kNumBoxVertices = 8;
constexpr uint32_t kMaxIndices = kMaxDebugNumBox * kBaseBoxIndices.size();

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

DebugRenderBatch::DebugRenderBatch() :
    vertices{ kMaxDebugNumBox * kNumBoxVertices },
    indices{ kMaxIndices } {
    // initialize box batching
    VertexAttribute attributes[] = { {3, PrimitiveVertexType::kFloat} };

    vertex_array.AddDynamicBuffer(vertices.GetCapacityBytes(), attributes);
    vertex_array.SetIndexBuffer(IndexBuffer(indices.GetCapacity()));

    vertex_array.Unbind();
}

void DebugRenderBatch::UploadBatchedData() {
    DebugVertexArrayScope bind_array_scope{ vertex_array };

    VertexBuffer& vertex_buffer = vertex_array.GetVertexBufferAt(0);
    vertex_buffer.UpdateVertices(vertices.GetRawData(), vertices.GetSizeBytes());

    IndexBuffer& index_buffer = vertex_array.GetIndexBuffer();
    index_buffer.UpdateIndices(indices.GetRawData(), indices.GetSize());
}

void DebugRenderBatch::FlushDraw(Shader& shader) {
    Renderer::Submit(shader, indices.GetSize(), vertex_array, glm::mat4{ 1.0 }, RenderPrimitive::kLines);
    vertices.ResetPtrToStart();
    indices.ResetPtrToStart();
    last_index_number = 0;
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform) {
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

    for (const glm::vec3& vertex : box_vertices) {
        vertices.AddInstance(transform * glm::vec4{ vertex, 1.0f });
    }

    for (uint32_t index : kBaseBoxIndices) {
        indices.AddInstance(index + last_index_number);
    }

    last_index_number += box_vertices.size();
}

bool DebugRenderBatch::CanBatchAnotherMesh(uint32_t num_indices) const {
    return last_index_number + num_indices < kMaxIndices && vertices.GetSize() < vertices.GetCapacity();
}

bool DebugRenderBatch::HasBatchedAnyPrimitive() const {
    return vertices.GetSize() > 0;
}
