#include "static_mesh_component.h"

StaticMeshComponent::StaticMeshComponent(const std::shared_ptr<StaticMesh>& mesh) :
    Mesh{mesh}
{
}

void StaticMeshComponent::Draw(const glm::mat4& worldTransform) const
{
    if (Mesh)
    {
        Mesh->Render(worldTransform);
    }
}