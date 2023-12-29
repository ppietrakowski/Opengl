#pragma once

#include "shader.h"
#include "vertex_array.h"
#include "material.h"

#include <filesystem>
#include <memory>

struct StaticMeshVertex
{
    glm::vec3 Position{0, 0,0};
    glm::vec3 Normal{0, 0, 0};
    glm::vec2 TextureCoords{0, 0};
    int32_t TextureId{0};

    static inline constexpr VertexAttribute kDataFormat[4] = {{3, PrimitiveVertexType::kFloat}, 
        {3, PrimitiveVertexType::kFloat}, {2, PrimitiveVertexType::kFloat}, {1, PrimitiveVertexType::kInt}};

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
        Position{position},
        Normal{normal},
        TextureCoords{textureCoords}
    {
    }

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
};

class StaticMesh
{
public:
    StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material);

public:
    void Render(const glm::mat4& transform) const;
    void Render(const Material& overrideMaterial, const glm::mat4& transform) const;

    int32_t GetNumPolygons() const;
    int32_t GetNumTriangles() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const
    {
        return m_MeshName;
    }

    std::vector<std::string> TexturePaths;
    std::shared_ptr<Material> MainMaterial;

private:
    std::shared_ptr<IVertexArray> m_VertexArray;
    int32_t m_NumTriangles;


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

inline int32_t StaticMesh::GetNumPolygons() const
{
    return m_NumTriangles;
}

inline int32_t StaticMesh::GetNumTriangles() const
{
    return m_NumTriangles;
}