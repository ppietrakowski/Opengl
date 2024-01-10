#include "static_mesh_component.h"

StaticMeshComponent::StaticMeshComponent(const std::shared_ptr<StaticMesh>& mesh) :
    UsedStaticMesh{mesh}
{
}

void StaticMeshComponent::Draw(const glm::mat4& worldTransform) const
{
    ASSERT(UsedStaticMesh);
    UsedStaticMesh->Render(worldTransform);
}