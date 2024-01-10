#include "debug_render_batch.h"
#include "renderer.h"
#include <array>

// Predefined box indices (base for offsets for box batching)
static const std::array<std::uint32_t, 24> BaseBoxIndices =
{
    0u, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

DebugRenderBatch::DebugRenderBatch() :
    m_InstanceDraw{std::array{VertexAttribute{3, PrimitiveVertexType::Float}, VertexAttribute{4, PrimitiveVertexType::Float}}}
{
}

void DebugRenderBatch::FlushDraw(Material& material)
{
    m_InstanceDraw.Draw(glm::mat4{1.0f}, material, RenderPrimitive::Lines);
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxMin, glm::vec3 boxMax, const Transform& transform)
{
    AddBoxInstance(boxMin, boxMax, transform, glm::vec4{0, 0, 0, 1});
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxMin, glm::vec3 boxMax, const Transform& transform, const glm::vec4& color)
{

    if (!Renderer::IsVisibleToCamera(transform.Position, boxMin, boxMax))
    {
        return;
    }

    std::array<DebugVertex, 8> boxVertices = {
        DebugVertex{glm::vec3{boxMin[0], boxMin[1], boxMin[2]}, color},
        DebugVertex{glm::vec3{boxMax[0], boxMin[1], boxMin[2]}, color},
        DebugVertex{glm::vec3{boxMax[0], boxMax[1], boxMin[2]}, color},
        DebugVertex{glm::vec3{boxMin[0], boxMax[1], boxMin[2]}, color},

        DebugVertex{glm::vec3{boxMin[0], boxMin[1], boxMax[2]}, color},
        DebugVertex{glm::vec3{boxMax[0], boxMin[1], boxMax[2]}, color},
        DebugVertex{glm::vec3{boxMax[0], boxMax[1], boxMax[2]}, color},
        DebugVertex{glm::vec3{boxMin[0], boxMax[1], boxMax[2]}, color}
    };

    m_InstanceDraw.QueueDraw(InstanceInfo<DebugVertex>{boxVertices, BaseBoxIndices, transform});
}

