#pragma once

#include "shader.h"
#include "vertex_array.h"
#include "material.h"

#include "box.h"

#include <filesystem>
#include <memory>

struct StaticMeshVertex
{
    glm::vec3 Position{0, 0,0};
    glm::vec3 Normal{0, 0, 0};
    glm::vec2 TextureCoords{0, 0};
    int TextureId{0};

    static inline constexpr VertexAttribute DataFormat[4] = {{3, PrimitiveVertexType::Float},
        {3, PrimitiveVertexType::Float}, {2, PrimitiveVertexType::Float}, {1, PrimitiveVertexType::Int}};

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords, int textureId);

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
};

class StaticMesh
{
    friend class InstancedMesh;

public:
    StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material);

public:
    int GetNumPolygons() const;
    int GetNumTriangles() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const;

    const VertexArray& GetVertexArray() const;

    std::shared_ptr<Material> MainMaterial;
    std::vector<StaticMeshVertex> Vertices;
    std::vector<uint32_t> Indices;
    std::vector<std::string> TextureNames;

    Box GetBoundingBox() const;

private:
    std::shared_ptr<VertexArray> m_VertexArray;
    int m_NumTriangles;

    Box m_BoundingBox;
    std::string m_MeshName;
};

FORCE_INLINE StaticMeshVertex::StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords, int textureId) :
    Position{position},
    Normal{normal},
    TextureCoords{textureCoords},
    TextureId{textureId}
{
}

FORCE_INLINE const glm::vec3& StaticMesh::GetBBoxMin() const
{
    return m_BoundingBox.MinBounds;
}

FORCE_INLINE const glm::vec3& StaticMesh::GetBBoxMax() const
{
    return m_BoundingBox.MaxBounds;
}

FORCE_INLINE int StaticMesh::GetNumPolygons() const
{
    return m_NumTriangles;
}

FORCE_INLINE int StaticMesh::GetNumTriangles() const
{
    return m_NumTriangles;
}

FORCE_INLINE std::string_view StaticMesh::GetName() const
{
    return m_MeshName;
}

FORCE_INLINE const VertexArray& StaticMesh::GetVertexArray() const
{
    return *m_VertexArray;
}

FORCE_INLINE Box StaticMesh::GetBoundingBox() const
{
    return m_BoundingBox;
}