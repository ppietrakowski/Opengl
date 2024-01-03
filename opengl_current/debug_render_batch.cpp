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

constexpr int32_t kMaxDebugNumBox = 100;
constexpr int32_t kNumBoxVertices = 8;
constexpr int32_t kMaxIndices = kMaxDebugNumBox * STD_ARRAY_NUM_ELEMENTS(kBaseBoxIndices);

/* Just does bind and unbind within scope */
struct DebugVertexArrayScope
{
    IVertexArray* Target;

    DebugVertexArrayScope(IVertexArray& target) :
        Target{&target}
    {
        target.Bind();
    }

    ~DebugVertexArrayScope()
    {
        Target->Unbind();
    }
};

DebugRenderBatch::DebugRenderBatch() :
    Vertices{kMaxDebugNumBox * kNumBoxVertices},
    Indices{kMaxIndices},
    VertexArray{IVertexArray::Create()}
{
    // initialize box batching
    VertexAttribute attributes[] = {{3, PrimitiveVertexType::kFloat}};

    VertexArray->AddDynamicBuffer(Vertices.GetCapacityBytes(), attributes);
    VertexArray->SetIndexBuffer(IIndexBuffer::CreateEmpty(Indices.GetCapacity()));

    VertexArray->Unbind();
}

void DebugRenderBatch::UploadBatchedData()
{
    DebugVertexArrayScope bindArrayScope{*VertexArray};

    std::shared_ptr<IVertexBuffer> vertexBuffer = VertexArray->GetVertexBufferAt(0);
    vertexBuffer->UpdateVertices(Vertices.GetRawData(), BufferSize{Vertices.GetSizeBytes()});

    std::shared_ptr<IIndexBuffer> indexBuffer = VertexArray->GetIndexBuffer();
    indexBuffer->UpdateIndices(Indices.GetRawData(), Indices.GetSize());
}

void DebugRenderBatch::FlushDraw(IShader& shader)
{
    Renderer::Submit(shader, Indices.GetSize(), *VertexArray, glm::mat4{1.0}, RenderPrimitive::kLines);
    Vertices.ResetPtrToStart();
    Indices.ResetPtrToStart();
    LastIndexNumber = 0;
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
        Vertices.AddInstance(transform * glm::vec4{vertex, 1.0f});
    }

    for (uint32_t index : kBaseBoxIndices)
    {
        Indices.AddInstance(index + LastIndexNumber);
    }

    int32_t maxNumVertices = STD_ARRAY_NUM_ELEMENTS(boxVertices);
    LastIndexNumber += maxNumVertices;
}

bool DebugRenderBatch::CanBatchAnotherMesh(int32_t numIndices) const
{
    return LastIndexNumber + numIndices < kMaxIndices && Vertices.GetSize() < Vertices.GetCapacity();
}

bool DebugRenderBatch::HasBatchedAnyPrimitive() const
{
    return Vertices.GetSize() > 0;
}
