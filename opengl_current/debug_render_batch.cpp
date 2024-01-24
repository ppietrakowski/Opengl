#include "debug_render_batch.h"
#include "renderer.h"
#include <array>

// Predefined box indices (base for offsets for box batching)
static const std::array<uint32_t, 24> BaseBoxIndices =
{
    0u, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

DebugRenderBatch::DebugRenderBatch(std::shared_ptr<Shader> shader) :
    m_BatchBase{std::array{VertexAttribute{3, PrimitiveVertexType::Float}, VertexAttribute{1, PrimitiveVertexType::UnsignedInt}}}
{

    m_Material = std::make_unique<Material>(shader);
}

void DebugRenderBatch::FlushDraw()
{
    m_BatchBase.DrawLines(glm::mat4{1.0f}, *m_Material);
}

void DebugRenderBatch::AddLineInstance(const Line& line, const Transform& transform, const glm::vec4& color)
{
    RgbaColor packedColor(color);

    std::array vertices = {
        DebugVertex{line.StartPos, packedColor},
        DebugVertex{line.EndPos, packedColor},
    };

    const uint32_t LineIndices[] = {0, 1};
    m_BatchBase.QueueDraw(BatchGeometryInfo<DebugVertex>{vertices, LineIndices, transform});
}

void DebugRenderBatch::AddBoxInstance(const Box& box, const Transform& transform, const glm::vec4& color)
{
    RgbaColor packedColor(color);

    std::array<DebugVertex, 8> boxVertices = {
        DebugVertex{glm::vec3{box.MinBounds[0], box.MinBounds[1], box.MinBounds[2]}, packedColor},
        DebugVertex{glm::vec3{box.MaxBounds[0], box.MinBounds[1], box.MinBounds[2]}, packedColor},
        DebugVertex{glm::vec3{box.MaxBounds[0], box.MaxBounds[1], box.MinBounds[2]}, packedColor},
        DebugVertex{glm::vec3{box.MinBounds[0], box.MaxBounds[1], box.MinBounds[2]}, packedColor},

        DebugVertex{glm::vec3{box.MinBounds[0], box.MinBounds[1], box.MaxBounds[2]}, packedColor},
        DebugVertex{glm::vec3{box.MaxBounds[0], box.MinBounds[1], box.MaxBounds[2]}, packedColor},
        DebugVertex{glm::vec3{box.MaxBounds[0], box.MaxBounds[1], box.MaxBounds[2]}, packedColor},
        DebugVertex{glm::vec3{box.MinBounds[0], box.MaxBounds[1], box.MaxBounds[2]}, packedColor}
    };

    m_BatchBase.QueueDraw(BatchGeometryInfo<DebugVertex>{boxVertices, BaseBoxIndices, transform});
}

