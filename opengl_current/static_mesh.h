#pragma once

#include "shader.h"
#include "vertex_array.h"
#include "material.h"
#include "instancing.h"

#include <filesystem>
#include <memory>

struct StaticMeshVertex
{
    glm::vec3 Position{0, 0,0};
    glm::vec3 Normal{0, 0, 0};
    glm::vec2 TextureCoords{0, 0};
    std::int32_t TextureId{0};

    static inline constexpr VertexAttribute DataFormat[4] = {{3, PrimitiveVertexType::Float},
        {3, PrimitiveVertexType::Float}, {2, PrimitiveVertexType::Float}, {1, PrimitiveVertexType::Int}};

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords, std::int32_t textureId) :
        Position{position},
        Normal{normal},
        TextureCoords{textureCoords},
        TextureId{textureId}
    {
    }

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
};

template<>
struct InstanceCreator<StaticMeshVertex>
{
    static StaticMeshVertex CreateInstanceFrom(const StaticMeshVertex& vertex, const glm::mat4& transform, std::int32_t textureId)
    {
        StaticMeshVertex v{vertex};
        glm::mat3 normal_matrix = transform;
        v.Position = transform * glm::vec4{v.Position,1};
        v.Normal = normal_matrix * v.Normal;
        v.TextureId = textureId;
        return v;
    }
};

class StaticMesh
{
public:
    StaticMesh(const std::filesystem::path& file_path, const std::shared_ptr<Material>& material);

public:
    void Render(const glm::mat4& transform) const;
    void Render(const Material& overrideMaterial, const glm::mat4& transform) const;

    std::int32_t GetNumPolygons() const;
    std::int32_t GetNumTriangles() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const
    {
        return m_MeshName;
    }

    std::vector<std::string> TexturePaths;
    std::shared_ptr<Material> MainMaterial;
    std::vector<StaticMeshVertex> Vertices;
    std::vector<std::uint32_t> Indices;

private:
    std::shared_ptr<VertexArray> m_VertexArray;
    std::int32_t m_NumTriangles;

    glm::vec3 m_BboxMin;
    glm::vec3 m_BboxMax;
    std::string m_MeshName;
};

inline const glm::vec3& StaticMesh::GetBBoxMin() const
{
    return m_BboxMin;
}

inline const glm::vec3& StaticMesh::GetBBoxMax() const
{
    return m_BboxMax;
}

inline std::int32_t StaticMesh::GetNumPolygons() const
{
    return m_NumTriangles;
}

inline std::int32_t StaticMesh::GetNumTriangles() const
{
    return m_NumTriangles;
}