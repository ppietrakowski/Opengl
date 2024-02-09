#include "InstancedMeshComponent.hpp"

void InstancedMeshComponent::Draw(const glm::mat4& transform) const
{
    TargetInstancedMesh->Draw(transform);
}
