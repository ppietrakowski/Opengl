#include "Mesh.h"
#include "Renderer.h"

namespace
{
    void FindAabCollision(std::span<const StaticMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
    {
        outBoxMin = glm::vec3{ std::numeric_limits<float>::max() };
        outBoxMax = glm::vec3{ std::numeric_limits<float>::min() };

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            const glm::vec3* vertex = &vertices[i].Position;

            if (vertex->x < outBoxMin.x)
            {
                outBoxMin.x = vertex->x;
            }
            if (vertex->y < outBoxMin.y)
            {
                outBoxMin.y = vertex->y;
            }
            if (vertex->z < outBoxMin.z)
            {
                outBoxMin.z = vertex->z;
            }

            if (vertex->x > outBoxMax.x)
            {
                outBoxMax.x = vertex->x;
            }
            if (vertex->y > outBoxMax.y)
            {
                outBoxMax.y = vertex->y;
            }
            if (vertex->z > outBoxMax.z)
            {
                outBoxMax.z = vertex->z;
            }
        }
    }
}
StaticMesh::StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material) :
    _material{ material }
{
    StaticMeshImporter importer(filePath);

    if (importer.HasErrorOccured())
    {
        throw importer.GetError();
    }

    std::span<const StaticMeshVertex> loadedVertices = importer.GetVertices();
    std::span<const unsigned int> loadedIndices = importer.GetIndices();

    VertexBuffer vertexBuffer(loadedVertices.data(),
        static_cast<unsigned int>(loadedVertices.size_bytes()));

    IndexBuffer indexBuffer(loadedIndices.data(),
        static_cast<unsigned int>(loadedIndices.size()));

    _vertexArray.AddBuffer(std::move(vertexBuffer), StaticMeshVertex::DataFormat);
    _vertexArray.SetIndexBuffer(std::move(indexBuffer));

    _numTriangles = static_cast<unsigned int>(importer.GetIndices().size()) / 3;

    _indices.resize(importer.GetNumIndices());
    _vertices.resize(importer.GetNumVertices());

    std::copy(loadedVertices.begin(), loadedVertices.end(), _vertices.begin());
    std::copy(loadedIndices.begin(), loadedIndices.end(), _indices.begin());

    _meshName = importer.GetModelName();
    FindAabCollision(importer.GetVertices(), _bboxMin, _bboxMax);
}


void StaticMesh::Render(const glm::mat4& transform) const
{
    Renderer::Submit(*_material, _vertexArray, transform);
}

void StaticMesh::Render(const Material& overrideMaterial, const glm::mat4& transform) const
{
    Renderer::Submit(overrideMaterial, _vertexArray, transform);
}