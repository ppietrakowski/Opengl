#pragma once

#include "Shader.h"
#include "VertexArray.h"
#include "MeshLoader.h"

#include <filesystem>
#include <memory>

#include "Material.h"

class StaticMesh
{
public:
    StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material);

public:
    void Render(const glm::mat4& transform) const;
    void Render(const Material &overrideMaterial, const glm::mat4& transform) const;

    std::uint32_t GetNumPolygons() const;
    std::uint32_t GetNumTriangles() const;

    std::span<const StaticMeshVertex> GetVertices() const;
    std::span<const std::uint32_t> GetIndices() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const { return _meshName; }

    const std::shared_ptr<Material>& GetMaterial() const { return _material; }

private:
    VertexArray _vertexArray;
    std::uint32_t _numTriangles;

    std::vector<StaticMeshVertex> _vertices;
    std::vector<std::uint32_t> _indices;
    std::shared_ptr<Material> _material;

    glm::vec3 _bboxMin;
    glm::vec3 _bboxMax;
    std::string _meshName;
};

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax);

inline std::span<const StaticMeshVertex> StaticMesh::GetVertices() const
{
    return _vertices;
}

inline std::span<const std::uint32_t> StaticMesh::GetIndices() const
{
    return _indices;
}

inline const glm::vec3& StaticMesh::GetBBoxMin() const
{
    return _bboxMin;
}

inline const glm::vec3& StaticMesh::GetBBoxMax() const
{
    return _bboxMax;
}

inline std::uint32_t StaticMesh::GetNumPolygons() const
{
    return _numTriangles;
}

inline std::uint32_t StaticMesh::GetNumTriangles() const
{
    return _numTriangles;
}