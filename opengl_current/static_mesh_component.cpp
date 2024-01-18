#include "static_mesh_component.h"

StaticMeshComponent::StaticMeshComponent(const std::shared_ptr<StaticMesh>& mesh) :
    static_mesh{mesh}
{
}

void StaticMeshComponent::Draw(const glm::mat4& world_transform) const
{
    ASSERT(static_mesh);
    static_mesh->Render(world_transform);
}