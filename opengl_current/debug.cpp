#include "debug.h"
#include "render_command.h"
#include "level_interface.h"

struct DebugVertex
{
    glm::vec3 Position;

    // Packed Color to minimize size of updated data
    RgbaColor Color;
};

static const uint32_t BaseBoxIndices[24] =
{
    0u, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr int MaxNumDebugVertices = 2000;
constexpr int MaxNumDebugIndices = 3 * 2000;

glm::mat4 Debug::s_ProjectionViewMatrix{1.0f};

class DebugRendererBatch
{
public:
    DebugRendererBatch(std::shared_ptr<Shader> debugShader) :
        m_Material(std::make_shared<Material>(debugShader))
    {
        m_Material->bCullFaces = false;
        m_VertexArray = std::make_shared<VertexArray>();

        std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(static_cast<int>(MaxNumDebugVertices * sizeof(DebugVertex)));
        std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(MaxNumDebugIndices);

        m_Vertices.resize(MaxNumDebugVertices);
        m_Indices.resize(MaxNumDebugIndices);
        m_VertexArray->AddVertexBuffer(vertexBuffer, std::array{VertexAttribute{3, PrimitiveVertexType::Float}, VertexAttribute{1, PrimitiveVertexType::UnsignedInt}});
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    void AddBoxInstance(const Box& box, const Transform& transform, const glm::vec4& color)
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

        glm::mat4 combinedTransform = transform.CalculateTransformMatrix();
        AddGeometry(boxVertices, BaseBoxIndices, combinedTransform);
    }

    void AddLineInstance(const Line& line, const Transform& transform, const glm::vec4& color)
    {
        RgbaColor packedColor(color);

        std::array vertices = {
            DebugVertex{line.StartPos, packedColor},
            DebugVertex{line.EndPos, packedColor},
        };

        const uint32_t LineIndices[] = {0, 1};
        glm::mat4 combinedTransform = transform.CalculateTransformMatrix();
        AddGeometry(vertices, LineIndices, combinedTransform);
    }

    void AddRectInstance(glm::vec2 position, glm::vec2 size, const Transform& transform, const glm::vec4& color)
    {
        // Offset to add to prevent flickering when camera moves
        constexpr float FlickeringStopOffset = 0.01f;

        RgbaColor packedColor(color);

        // rect data initialized with screen space vertices
        std::array vertices = {
            DebugVertex{glm::vec3(position, 0), packedColor},
            DebugVertex{glm::vec3(position.x + size.x, position.y, FlickeringStopOffset), packedColor},
            DebugVertex{glm::vec3(position.x + size.x, position.y + size.y, FlickeringStopOffset), packedColor},
            DebugVertex{glm::vec3(position.x, position.y + size.y, FlickeringStopOffset), packedColor},
        };

        // DebugVertexBatch requires vertices to be in world space so project every point to world
        for (DebugVertex& vertex : vertices)
        {
            vertex.Position = LevelInterface::ProjectScreenToWorld(vertex.Position);
        }

        const uint32_t rectIndices[] = {0, 1, 1, 2, 2, 3, 3, 0};
        glm::mat4 combinedTransform = transform.CalculateTransformMatrix();
        AddGeometry(vertices, rectIndices, combinedTransform);
    }

    void FlushDraw()
    {
        m_VertexArray->Bind();
        UploadBufferData();

        std::shared_ptr<Shader> shader = m_Material->GetShader();

        shader->Use();
        shader->SetUniform("u_ProjectionView", Debug::GetProjectionViewMatrix());

        RenderCommand::DrawLines(m_VertexArray, m_NumDrawIndices);

        m_NumDrawIndices = 0;
        m_NumDrawVertices = 0;
        m_VertexArray->Unbind();
    }

private:
    std::vector<DebugVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    std::shared_ptr<VertexArray> m_VertexArray;

    int m_NumDrawVertices{0};
    int m_NumDrawIndices{0};
    std::shared_ptr<Material> m_Material;
    bool m_bBuffersDirty{false};

private:
    void UploadBufferData()
    {
        if (m_bBuffersDirty)
        {
            std::shared_ptr<VertexBuffer> vertexBuffer = m_VertexArray->GetVertexBufferAt(0);
            std::shared_ptr<IndexBuffer> indexBuffer = m_VertexArray->GetIndexBuffer();

            vertexBuffer->UpdateVertices(m_Vertices.data(), m_NumDrawVertices * sizeof(DebugVertex));
            indexBuffer->UpdateIndices(m_Indices.data(), m_NumDrawIndices);
            m_bBuffersDirty = false;
        }
    }

    void AddGeometry(std::span<const DebugVertex> vertices, std::span<const uint32_t> indices, const glm::mat4& transform)
    {
        for (uint32_t index : indices)
        {
            // set though indexing is faster than emplace_back so use it
            m_Indices[m_NumDrawIndices++] = index + m_NumDrawVertices;
        }

        for (const DebugVertex& vertex : vertices)
        {
            DebugVertex v = vertex;
            v.Position = transform * glm::vec4(vertex.Position, 1.0f);
            m_Vertices[m_NumDrawVertices++] = v;
        }

        m_bBuffersDirty = true;
    }
};

static DebugRendererBatch* s_DebugRenderBatch = nullptr;

void Debug::Quit()
{
    SafeDelete(s_DebugRenderBatch);
}

void Debug::BeginScene(const glm::mat4& projectionViewMatrix)
{
    s_ProjectionViewMatrix = projectionViewMatrix;
}

void Debug::DrawDebugBox(const Box& box, const Transform& transform, const glm::vec4& color)
{
    s_DebugRenderBatch->AddBoxInstance(box, transform, color);
}

void Debug::DrawDebugLine(const Line& line, const Transform& transform, const glm::vec4& color)
{
    s_DebugRenderBatch->AddLineInstance(line, transform, color);
}

void Debug::DrawDebugRect(const glm::vec2& position, const glm::vec2& size, const Transform& transform, const glm::vec4& color)
{
    s_DebugRenderBatch->AddRectInstance(position, size, transform, color);
}

void Debug::FlushDrawDebug()
{
    s_DebugRenderBatch->FlushDraw();
}

void Debug::InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader)
{
    s_DebugRenderBatch = new DebugRendererBatch(debugShader);
}
