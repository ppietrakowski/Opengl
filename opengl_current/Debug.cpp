#include "Debug.hpp"
#include "RenderCommand.hpp"
#include "LevelInterface.hpp"

#include "Material.hpp"

struct DebugVertex
{
    glm::vec3 Position;

    // Packed Color to minimize size of updated data
    RgbaColor Color;
};

static constexpr uint32_t BaseBoxIndices[24] =
{
    0u, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr int32_t MaxNumDebugVertices{2000};
constexpr int32_t MaxNumDebugIndices{3 * 2000};

glm::mat4 Debug::s_ProjectionViewMatrix{1.0f};

class DebugRendererBatch
{
public:
    DebugRendererBatch(std::shared_ptr<Shader> debugShader) :
        m_Material{std::make_shared<Material>(debugShader)}
    {
        m_Material->bCullFaces = false;
        m_VertexArray = std::make_shared<VertexArray>();

        auto vertexBuffer = std::make_shared<VertexBuffer>(static_cast<int>(MaxNumDebugVertices * sizeof(DebugVertex)));
        auto indexBuffer = std::make_shared<IndexBuffer>(MaxNumDebugIndices);

        m_Vertices.resize(MaxNumDebugVertices);
        m_Indices.resize(MaxNumDebugIndices);

        constexpr std::array<VertexAttribute, 2> DebugVertexDataFormat{VertexAttribute{3, PrimitiveVertexType::Float}, VertexAttribute{1, PrimitiveVertexType::UnsignedInt}};

        m_VertexArray->AddVertexBuffer(vertexBuffer, DebugVertexDataFormat);
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    void AddBoxInstance(const Box& box, const Transform& transform, const glm::vec4& color)
    {
        RgbaColor packedColor{color};

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
        RgbaColor packedColor{color};

        std::array vertices = {
            DebugVertex{line.StartPos, packedColor},
            DebugVertex{line.EndPos, packedColor},
        };

        constexpr uint32_t LineIndices[] = {0, 1};
        glm::mat4 combinedTransform = transform.CalculateTransformMatrix();
        AddGeometry(vertices, LineIndices, combinedTransform);
    }

    void AddRectInstance(glm::vec2 position, glm::vec2 size, const Transform& transform, const glm::vec4& color)
    {
        // Offset to add to prevent flickering when camera moves
        constexpr float FlickeringStopOffset = 0.5f;

        RgbaColor packedColor{color};

        // rect data initialized with screen space vertices
        std::array vertices = {
            DebugVertex{glm::vec3{position, 0}, packedColor},
            DebugVertex{glm::vec3{position.x + size.x, position.y, FlickeringStopOffset}, packedColor},
            DebugVertex{glm::vec3{position.x + size.x, position.y + size.y, FlickeringStopOffset}, packedColor},
            DebugVertex{glm::vec3{position.x, position.y + size.y, FlickeringStopOffset}, packedColor},
        };

        // DebugVertexBatch requires vertices to be in world space so project every point to world
        for (DebugVertex& vertex : vertices)
        {
            vertex.Position = LevelInterface::ProjectScreenToWorld(vertex.Position);
        }

        constexpr uint32_t RectIndices[] = {0, 1, 1, 2, 2, 3, 3, 0};
        glm::mat4 combinedTransform = transform.CalculateTransformMatrix();
        AddGeometry(vertices, RectIndices, combinedTransform);
    }

    void FlushDraw()
    {
        m_VertexArray->Bind();
        UploadBufferData();

        std::shared_ptr<Shader> shader = m_Material->GetShader();

        shader->Use();
        shader->SetUniform("u_ProjectionView", Debug::GetProjectionViewMatrix());

        RenderCommand::DrawLines(*m_VertexArray, m_NumDrawIndices);

        m_NumDrawIndices = 0;
        m_NumDrawVertices = 0;
        m_VertexArray->Unbind();
    }

private:
    std::vector<DebugVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    std::shared_ptr<VertexArray> m_VertexArray;

    int32_t m_NumDrawVertices{0};
    int32_t m_NumDrawIndices{0};
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
            m_Indices[m_NumDrawIndices] = index + m_NumDrawVertices;
            m_NumDrawIndices++;
        }

        for (const DebugVertex& vertex : vertices)
        {
            DebugVertex transformedVertex = vertex;
            transformedVertex.Position = transform * glm::vec4{vertex.Position, 1.0f};
            m_Vertices[m_NumDrawVertices] = transformedVertex;
            m_NumDrawVertices++;
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

void Debug::DrawDebugRect(glm::vec2 position, glm::vec2 size, const Transform& transform, const glm::vec4& color)
{
    s_DebugRenderBatch->AddRectInstance(position, size, transform, color);
}

void Debug::FlushDrawDebug()
{
    s_DebugRenderBatch->FlushDraw();
}

void Debug::InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader)
{
    SafeDelete(s_DebugRenderBatch);
    s_DebugRenderBatch = new DebugRendererBatch(debugShader);
}
