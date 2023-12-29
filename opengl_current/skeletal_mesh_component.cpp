#include "skeletal_mesh_component.h"

#include "renderer.h"

void SkeletalMeshComponent::Draw(const glm::mat4& worldTransform)
{
    if (bShouldDrawDebugBounds)
    {
        Renderer::DrawDebugBox(DebugBboxMin, DebugBboxMax, worldTransform);
    }

    Mesh->Draw(BoneTransforms, worldTransform);
}