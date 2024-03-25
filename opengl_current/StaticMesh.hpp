#pragma once

#include "Shader.hpp"
#include "VertexArray.hpp"
#include "Material.hpp"

#include "Box.hpp"

#include <filesystem>
#include <memory>

#include "Core.hpp"

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

/* Entry for one LOD for mesh. This class must be copy-constructible */
class StaticMeshEntry
{
    friend class InstancedMesh;
public:
    StaticMeshEntry(const std::vector<StaticMeshVertex>& vertices, const std::vector<uint32_t>& indices, const std::shared_ptr<Material>& material);

public:
    std::vector<StaticMeshVertex> Vertices;
    std::vector<uint32_t> Indices;

    const VertexArray& GetVertexArray() const
    {
        return m_VertexArray;
    }

    int GetNumIndices() const
    {
        return static_cast<int>(Indices.size());
    }

    const Material& GetMaterial() const
    {
        return *m_Material;
    }

    void UpdateMaterial(std::shared_ptr<Material> material)
    {
        m_Material = material;
    }

private:
    VertexArray m_VertexArray;
    std::shared_ptr<Material> m_Material;
    int m_NumTriangles;
};

class StaticMesh
{
    friend class InstancedMesh;
public:
    StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material);

public:
    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const;

    std::vector<std::string> TextureNames;

    Box GetBoundingBox() const;

    const StaticMeshEntry& GetStaticMeshEntry(int lod);

    int GetNumLods() const
    {
        return static_cast<int>(m_Entries.size());
    }

    void LoadLod(const std::string& filePath, int lod);

    void SetMaterial(std::shared_ptr<Material> material);

private:
    std::vector<StaticMeshEntry> m_Entries;
    Box m_BoundingBox;
    std::string m_MeshName;
    std::shared_ptr<Material> m_MainMaterial;
};

struct MeshKey
{
    std::string Name;
    int Lod{0};

    bool operator==(const MeshKey& other) const
    {
        return other.Name == Name && other.Lod == Lod;
    }
};

namespace std
{
    template<>
    struct hash<::MeshKey>
    {
        hash<std::string> Hash;

        size_t operator()(const MeshKey& key) const
        {
            return key.Lod * Hash(key.Name);
        }
    };
}

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

FORCE_INLINE std::string_view StaticMesh::GetName() const
{
    return m_MeshName;
}

FORCE_INLINE Box StaticMesh::GetBoundingBox() const
{
    return m_BoundingBox;
}

inline const StaticMeshEntry& StaticMesh::GetStaticMeshEntry(int lod)
{
    return m_Entries.at(lod);
}

inline void StaticMesh::SetMaterial(std::shared_ptr<Material> material)
{
    m_MainMaterial = material;
    for (StaticMeshEntry& entry : m_Entries)
    {
        entry.UpdateMaterial(material);
    }
}
