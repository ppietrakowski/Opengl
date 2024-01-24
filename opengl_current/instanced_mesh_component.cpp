#include "instanced_mesh_component.h"

void InstancedMeshComponent::Draw(const glm::mat4& transform) const
{
    TargetInstancedMesh->Draw(transform);
}
