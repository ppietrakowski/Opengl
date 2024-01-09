#pragma once

#include "static_mesh.h"
#include "buffer.h"
#include <glm/gtc/quaternion.hpp>

class InstancedMesh {
public:
    InstancedMesh(const StaticMesh& static_mesh);

    void Draw(const glm::mat4& transform, Material& material);

    void QueueDraw(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, std::int32_t texture_id);

private:
    Buffer<StaticMeshVertex> vertices_;
    Buffer<std::uint32_t> indices_;
    std::shared_ptr<VertexArray> vertex_array_;

    std::vector<StaticMeshVertex> base_vertices_;
    std::vector<std::uint32_t> base_indices_;
    std::int32_t start_index_{0};

private:
    void ResizeBuffers(std::int32_t maxVerticesSizeBytes, std::int32_t maxNumIndices);
};

