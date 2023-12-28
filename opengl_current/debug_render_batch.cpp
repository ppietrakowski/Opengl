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
constexpr uint32_t kMaxIndices = kMaxDebugNumBox * STD_ARRAY_NUM_ELEMENTS(kBaseBoxIndices);

/* Just does bind and unbind within scope */
struct DebugVertexArrayScope {
    IVertexArray* Target;

    DebugVertexArrayScope(IVertexArray& target) :
        Target{ &target } {
        target.Bind();
    }

    ~DebugVertexArrayScope() {
        Target->Unbind();
    }
};

DebugRenderBatch::DebugRenderBatch() :
    m_Vertices{ kMaxDebugNumBox * kNumBoxVertices },
    m_Indices{ kMaxIndices },
    m_VertexArray{ IVertexArray::Create() } {
    // initialize box batching
    VertexAttribute attributes[] = { {3, PrimitiveVertexType::kFloat} };

    m_VertexArray->AddDynamicBuffer(m_Vertices.GetCapacityBytes(), attributes);
    m_VertexArray->SetIndexBuffer(IIndexBuffer::CreateEmpty(m_Indices.GetCapacity()));

    m_VertexArray->Unbind();
}

void DebugRenderBatch::UploadBatchedData() {
    DebugVertexArrayScope bindArrayScope{ *m_VertexArray };

    std::shared_ptr<IVertexBuffer> vertexBuffer = m_VertexArray->GetVertexBufferAt(0);
    vertexBuffer->UpdateVertices(m_Vertices.GetRawData(), m_Vertices.GetSizeBytes());

    std::shared_ptr<IIndexBuffer> indexBuffer = m_VertexArray->GetIndexBuffer();
    indexBuffer->UpdateIndices(m_Indices.GetRawData(), m_Indices.GetSize());
}

void DebugRenderBatch::FlushDraw(IShader& shader) {
    Renderer::Submit(shader, m_Indices.GetSize(), *m_VertexArray, glm::mat4{ 1.0 }, RenderPrimitive::kLines);
    m_Vertices.ResetPtrToStart();
    m_Indices.ResetPtrToStart();
    m_LastIndexNumber = 0;
}

void DebugRenderBatch::AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform) {
    if (!CanBatchAnotherMesh(ARRAY_NUM_ELEMENTS(kBaseBoxIndices))) {
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


    for (const glm::vec3& vertex : boxVertices) {
        m_Vertices.AddInstance(transform * glm::vec4{ vertex, 1.0f });
    }

    for (uint32_t index : kBaseBoxIndices) {
        m_Indices.AddInstance(index + m_LastIndexNumber);
    }

    uint32_t maxNumVertices = STD_ARRAY_NUM_ELEMENTS(boxVertices);
    m_LastIndexNumber += maxNumVertices;
}

bool DebugRenderBatch::CanBatchAnotherMesh(uint32_t numIndices) const {
    return m_LastIndexNumber + numIndices < kMaxIndices && m_Vertices.GetSize() < m_Vertices.GetCapacity();
}

bool DebugRenderBatch::HasBatchedAnyPrimitive() const {
    return m_Vertices.GetSize() > 0;
}
