#pragma once

#include "Shader.h"
#include "VertexArray.h"

#include <filesystem>
#include <memory>

#include "Material.h"

struct StaticMeshVertex
{
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };
    std::uint32_t ID{ 0 };

    static inline constexpr VertexAttribute DataFormat[4] = { {3, PrimitiveVertexType::Float}, {3, PrimitiveVertexType::Float}, {2, PrimitiveVertexType::Float}, {1, PrimitiveVertexType::UnsignedInt} };

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
        Position{ position },
        Normal{ normal },
        TextureCoords{ textureCoords }
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
    void Render(const Material &overrideMaterial, const glm::mat4& transform) const;

    std::uint32_t GetNumPolygons() const;
    std::uint32_t GetNumTriangles() const;

    std::span<const StaticMeshVertex> GetVertices() const;
    std::span<const std::uint32_t> GetIndices() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const { return _meshName; }

    const std::shared_ptr<Material>& GetMaterial() const { return _material; }

    std::vector<std::string> TexturePaths;

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