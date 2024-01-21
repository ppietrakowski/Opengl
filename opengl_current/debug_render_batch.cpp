#include "debug_render_batch.h"
#include "renderer.h"
#include <array>

// Predefined box indices (base for offsets for box batching)
static const std::array<uint32_t, 24> kBaseBoxIndices =
{
    0u, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

DebugRenderBatch::DebugRenderBatch() :
    batch_base_{std::array{VertexAttribute{3, PrimitiveVertexType::kFloat}, VertexAttribute{1, PrimitiveVertexType::kUnsignedInt}}} {
}

void DebugRenderBatch::FlushDraw(Material& material) {
    batch_base_.DrawLines(glm::mat4{1.0f}, material);
}

void DebugRenderBatch::AddLineInstance(const glm::vec3& start_pos, const glm::vec3& end_pos, const Transform& transform, const glm::vec4& color) {
    RgbaColor packed_color(color);

    std::array vertices = {
        DebugVertex{start_pos, packed_color},
        DebugVertex{end_pos, packed_color},
    };

    uint32_t kLineIndices[] = {0, 1};
    batch_base_.QueueDraw(BatchGeometryInfo<DebugVertex>{vertices, kLineIndices, transform});
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 box_min, glm::vec3 box_max, const Transform& transform, const glm::vec4& color) {
    if (!Renderer::IsVisibleToCamera(transform.position, box_min, box_max)) {
        return;
    }

    RgbaColor packed_color(color);

    std::array<DebugVertex, 8> box_vertices = {
        DebugVertex{glm::vec3{box_min[0], box_min[1], box_min[2]}, packed_color},
        DebugVertex{glm::vec3{box_max[0], box_min[1], box_min[2]}, packed_color},
        DebugVertex{glm::vec3{box_max[0], box_max[1], box_min[2]}, packed_color},
        DebugVertex{glm::vec3{box_min[0], box_max[1], box_min[2]}, packed_color},

        DebugVertex{glm::vec3{box_min[0], box_min[1], box_max[2]}, packed_color},
        DebugVertex{glm::vec3{box_max[0], box_min[1], box_max[2]}, packed_color},
        DebugVertex{glm::vec3{box_max[0], box_max[1], box_max[2]}, packed_color},
        DebugVertex{glm::vec3{box_min[0], box_max[1], box_max[2]}, packed_color}
    };

    batch_base_.QueueDraw(BatchGeometryInfo<DebugVertex>{box_vertices, kBaseBoxIndices, transform});
}

