#include "instanced_mesh.h"
#include "logging.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

InstancedMesh::InstancedMesh(const std::shared_ptr<StaticMesh>& static_mesh) :
    instance_draw_(StaticMeshVertex::kDataFormat),
    base_vertices_{static_mesh->vertices},
    base_indices_{static_mesh->indices},
    static_mesh_{static_mesh} {
}

void InstancedMesh::Draw(const glm::mat4& transform, Material& material) {
    instance_draw_.Draw(transform, material);
}

void InstancedMesh::QueueDraw(const Transform& transform, std::int32_t texture_id) {
    // prevent instancing something that's not visible
    if (!Renderer::IsVisibleToCamera(transform.position, static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax())) {
        return;
    }

    instance_draw_.QueueDraw(InstanceInfo<StaticMeshVertex>{base_vertices_, base_indices_, transform}, texture_id);
}
