#include "skeletal_mesh_component.h"

#include "renderer.h"

SkeletalMeshComponent::SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh):
    Mesh{mesh}
{
    std::vector<std::string> animations = mesh->GetAnimationNames();
    AnimationName = animations.back();
    BoneTransforms.resize(mesh->GetNumBones(), glm::identity<glm::mat4>());

    DebugBboxMin = Mesh->GetBboxMin();
    DebugBboxMax = Mesh->GetBboxMax();
}

void SkeletalMeshComponent::Draw(const glm::mat4& worldTransform)
{
    if (bShouldDrawDebugBounds)
    {
        Renderer::DrawDebugBox(DebugBboxMin, DebugBboxMax, worldTransform);
    }

    Mesh->Draw(BoneTransforms, worldTransform);
}