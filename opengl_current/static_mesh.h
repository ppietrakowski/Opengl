#pragma once

#include "shader.h"
#include "vertex_array.h"
#include "material.h"

#include <filesystem>
#include <memory>

struct StaticMeshVertex {
    glm::vec3 position{ 0, 0,0 };
    glm::vec3 normal{ 0, 0, 0 };
    glm::vec2 texture_coords{ 0, 0 };
    uint32_t texture_id{ 0 };

    static inline constexpr VertexAttribute data_format[4] = { {3, PrimitiveVertexType::kFloat}, {3, PrimitiveVertexType::kFloat}, {2, PrimitiveVertexType::kFloat}, {1, PrimitiveVertexType::kUnsignedInt} };

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coords) :
        position{ position },
        normal{ normal },
        texture_coords{ texture_coords } {}

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
};

class StaticMesh {
public:
    StaticMesh(const std::filesystem::path& file_path, const std::shared_ptr<Material>& material);

public:
    void Render(const glm::mat4& transform) const;
    void Render(const Material& override_material, const glm::mat4& transform) const;

    uint32_t GetNumPolygons() const;
    uint32_t GetNumTriangles() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const {
        return mesh_name_;
    }

    const std::shared_ptr<Material>& GetMaterial() const {
        return material_;
    }

    std::vector<std::string> texture_paths;

private:
    VertexArray vertex_array_;
    uint32_t num_triangles_;

    std::shared_ptr<Material> material_;

    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    std::string mesh_name_;
};

inline const glm::vec3& StaticMesh::GetBBoxMin() const {
    return bbox_min_;
}

inline const glm::vec3& StaticMesh::GetBBoxMax() const {
    return bbox_max_;
}

inline uint32_t StaticMesh::GetNumPolygons() const {
    return num_triangles_;
}

inline uint32_t StaticMesh::GetNumTriangles() const {
    return num_triangles_;
}