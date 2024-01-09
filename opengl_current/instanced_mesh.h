#pragma once

#include "static_mesh.h"
#include "buffer.h"
#include "instancing.h"

#include <glm/gtc/quaternion.hpp>

class InstancedMesh {
public:
    InstancedMesh(const std::shared_ptr<StaticMesh>& static_mesh);

    void Draw(const glm::mat4& transform, Material& material);

    void QueueDraw(const Transform& transform, std::int32_t texture_id);

    const StaticMesh& GetMesh() const {
        return *static_mesh_;
    }
private: 
    std::vector<StaticMeshVertex> base_vertices_;
    std::vector<std::uint32_t> base_indices_;
    InstanceBase<StaticMeshVertex> instance_draw_;
    std::shared_ptr<StaticMesh> static_mesh_;
};

