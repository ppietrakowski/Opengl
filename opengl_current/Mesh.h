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

    unsigned int GetNumPolygons() const;
    unsigned int GetNumTriangles() const;

    std::span<const StaticMeshVertex> GetVertices() const;
    std::span<const unsigned int> GetIndices() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const { return _meshName; }

    const std::shared_ptr<Material>& GetMaterial() const { return _material; }

private:
    VertexArray _vertexArray;
    unsigned int _numTriangles;

    std::vector<StaticMeshVertex> _vertices;
    std::vector<unsigned int> _indices;
    std::shared_ptr<Material> _material;

    glm::vec3 _bboxMin;
    glm::vec3 _bboxMax;
    std::string _meshName;
};


inline std::span<const StaticMeshVertex> StaticMesh::GetVertices() const
{
    return _vertices;
}

inline std::span<const unsigned int> StaticMesh::GetIndices() const
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

inline unsigned int StaticMesh::GetNumPolygons() const
{
    return _numTriangles;
}

inline unsigned int StaticMesh::GetNumTriangles() const
{
    return _numTriangles;
}