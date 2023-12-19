#include "Mesh.h"
#include "Renderer.h"

void FindAabCollision(std::span<const StaticMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
{
    // assume mesh has infinite bounds
    outBoxMin = glm::vec3{ std::numeric_limits<float>::max() };
    outBoxMax = glm::vec3{ std::numeric_limits<float>::min() };

    for (std::size_t i = 0; i < vertices.size(); ++i)
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

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
{
    // assume mesh has infinite bounds
    outBoxMin = glm::vec3{ std::numeric_limits<float>::max() };
    outBoxMax = glm::vec3{ std::numeric_limits<float>::min() };

    for (std::size_t i = 0; i < vertices.size(); ++i)
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


StaticMesh::StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material) :
    _material{ material }
{
    StaticMeshImporter importer{ filePath };

    if (importer.HasErrorOccured())
    {
        throw importer.GetError();
    }

    std::span<const StaticMeshVertex> loadedVertices = importer.GetVertices();
    std::span<const std::uint32_t> loadedIndices = importer.GetIndices();

    IndexBuffer indexBuffer(loadedIndices.data(),
        static_cast<std::uint32_t>(loadedIndices.size()));

    _vertexArray.AddBuffer(loadedVertices, StaticMeshVertex::DataFormat);
    _vertexArray.SetIndexBuffer(std::move(indexBuffer));

    _numTriangles = static_cast<std::uint32_t>(loadedIndices.size()) / 3;

    _indices.resize(loadedIndices.size());
    _vertices.resize(loadedVertices.size());

    std::copy(loadedVertices.begin(), loadedVertices.end(), _vertices.begin());
    std::copy(loadedIndices.begin(), loadedIndices.end(), _indices.begin());

    _meshName = importer.GetModelName();
    FindAabCollision(loadedVertices, _bboxMin, _bboxMax);
}


void StaticMesh::Render(const glm::mat4& transform) const
{
    Renderer::Submit(*_material, _vertexArray, transform);
}

void StaticMesh::Render(const Material& overrideMaterial, const glm::mat4& transform) const
{
    Renderer::Submit(overrideMaterial, _vertexArray, transform);
}
