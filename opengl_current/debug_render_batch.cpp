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

DebugRenderBatch::DebugRenderBatch() :
    instance_draw_{std::array{VertexAttribute{3, PrimitiveVertexType::kFloat}, VertexAttribute{4, PrimitiveVertexType::kFloat}}} {
}

void DebugRenderBatch::FlushDraw(Material& material) {
    instance_draw_.Draw(glm::mat4{1.0f}, material, RenderPrimitive::kLines);
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform) {
    AddBoxInstance(boxmin, boxmax, transform, glm::vec4{0, 0, 0, 1});
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform, const glm::vec4& color) {

    if (!Renderer::IsVisibleToCamera(transform.position, boxmin, boxmax)) {
        return;
    }

    std::array<DebugVertex, 8> boxVertices = {
        DebugVertex{glm::vec3{boxmin[0], boxmin[1], boxmin[2]}, color},
        DebugVertex{glm::vec3{boxmax[0], boxmin[1], boxmin[2]}, color},
        DebugVertex{glm::vec3{boxmax[0], boxmax[1], boxmin[2]}, color},
        DebugVertex{glm::vec3{boxmin[0], boxmax[1], boxmin[2]}, color},

        DebugVertex{glm::vec3{boxmin[0], boxmin[1], boxmax[2]}, color},
        DebugVertex{glm::vec3{boxmax[0], boxmin[1], boxmax[2]}, color},
        DebugVertex{glm::vec3{boxmax[0], boxmax[1], boxmax[2]}, color},
        DebugVertex{glm::vec3{boxmin[0], boxmax[1], boxmax[2]}, color}
    };

    instance_draw_.QueueDraw(InstanceInfo<DebugVertex>{boxVertices, kBaseBoxIndices, transform});
}

