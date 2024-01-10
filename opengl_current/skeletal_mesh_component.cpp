#include "skeletal_mesh_component.h"

#include "renderer.h"

SkeletalMeshComponent::SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh) :
    UsedSkeletalMesh{mesh}
{
    std::vector<std::string> animations = mesh->GetAnimationNames();
    AnimationName = animations.back();
    BoneTransforms.resize(mesh->GetNumBones(), glm::identity<glm::mat4>());
}

void SkeletalMeshComponent::UpdateAnimation(float deltaSeconds)
{
    AnimationTime += deltaSeconds;
    UsedSkeletalMesh->GetAnimationFrames(AnimationTime, AnimationName, BoneTransforms);
}

void SkeletalMeshComponent::Draw(const glm::mat4& worldTransform)
{
    UsedSkeletalMesh->Draw(BoneTransforms, worldTransform);
}