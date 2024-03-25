#include "InstancedMeshComponent.hpp"

void InstancedMeshComponent::Draw(const glm::mat4& transform) const
{
    TargetInstancedMesh->Draw(transform);
}

Datapack InstancedMeshComponent::Archived() const
{
    Datapack p;
    p["NumInstances"] = static_cast<int>(Transforms.size());

    int index = 0;

    for (auto& transform : Transforms)
    {
        p["I" + std::to_string(index++)] = transform.Archived();
    }

    return p;
}
