#pragma once

#include "shader.h"
#include "vertex_array.h"
#include "material.h"
#include "batching.h"

#include "box.h"

#include <filesystem>
#include <memory>

struct StaticMeshVertex {
    glm::vec3 position{0, 0,0};
    glm::vec3 normal{0, 0, 0};
    glm::vec2 texture_coords{0, 0};
    int texture_id{0};
    
    static inline constexpr VertexAttribute kDataFormat[4] = {{3, PrimitiveVertexType::kFloat},
        {3, PrimitiveVertexType::kFloat}, {2, PrimitiveVertexType::kFloat}, {1, PrimitiveVertexType::kInt}};

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coords, int texture_id);

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
};

template<>
struct BatchVertexCreator<StaticMeshVertex> {
    static StaticMeshVertex CreateInstanceFrom(const StaticMeshVertex& vertex, const glm::mat4& transform, int texture_id) {
        StaticMeshVertex v{vertex};
        glm::mat3 normal_matrix = transform;
        v.position = transform * glm::vec4{v.position,1};
        v.normal = normal_matrix * v.normal;
        v.texture_id = texture_id;
        return v;
    }
};

class StaticMesh {
    friend class InstancedMesh;

public:
    StaticMesh(const std::filesystem::path& file_path, const std::shared_ptr<Material>& material);

public:
    int GetNumPolygons() const;
    int GetNumTriangles() const;

    const glm::vec3& GetBBoxMin() const;
    const glm::vec3& GetBBoxMax() const;

    std::string_view GetName() const;

    const VertexArray& GetVertexArray() const;

    std::shared_ptr<Material> main_material;
    std::vector<StaticMeshVertex> vertices;
    std::vector<uint32_t> indices;

    Box GetBoundingBox() const;

private:
    std::shared_ptr<VertexArray> vertex_array_;
    int num_triangles_;

    Box bounding_box_;
    std::string mesh_name_;
};

FORCE_INLINE StaticMeshVertex::StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coords, int texture_id) :
    position{position},
    normal{normal},
    texture_coords{texture_coords},
    texture_id{texture_id} {
}

FORCE_INLINE const glm::vec3& StaticMesh::GetBBoxMin() const {
    return bounding_box_.min_bounds;
}

FORCE_INLINE const glm::vec3& StaticMesh::GetBBoxMax() const {
    return bounding_box_.max_bounds;
}

FORCE_INLINE int StaticMesh::GetNumPolygons() const {
    return num_triangles_;
}

FORCE_INLINE int StaticMesh::GetNumTriangles() const {
    return num_triangles_;
}

FORCE_INLINE std::string_view StaticMesh::GetName() const {
    return mesh_name_;
}

FORCE_INLINE const VertexArray& StaticMesh::GetVertexArray() const {
    return *vertex_array_;
}

FORCE_INLINE Box StaticMesh::GetBoundingBox() const {
    return bounding_box_;
}