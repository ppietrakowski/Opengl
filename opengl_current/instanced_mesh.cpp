#include "instanced_mesh.h"
#include "logging.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

InstancedMesh::InstancedMesh(const StaticMesh& static_mesh) :
    instance_draw_(StaticMeshVertex::kDataFormat),
    base_vertices_{static_mesh.vertices},
    base_indices_{static_mesh.indices}
{
}

void InstancedMesh::Draw(const glm::mat4& transform, Material& material) {
    instance_draw_.Draw(transform, material);
}

void InstancedMesh::QueueDraw(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, std::int32_t texture_id) {
    instance_draw_.QueueDraw(base_vertices_, base_indices_, position, rotation, scale, texture_id);
}
