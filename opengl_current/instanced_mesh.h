#pragma once

#include "static_mesh.h"
#include "buffer.h"
#include "instancing.h"

#include <glm/gtc/quaternion.hpp>

class InstancedMesh {
public:
    InstancedMesh(const StaticMesh& static_mesh);

    void Draw(const glm::mat4& transform, Material& material);

    void QueueDraw(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, std::int32_t texture_id);

private:
    std::vector<StaticMeshVertex> base_vertices_;
    std::vector<std::uint32_t> base_indices_;
    InstanceBase<StaticMeshVertex> instance_draw_;
};

