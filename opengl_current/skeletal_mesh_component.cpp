#include "skeletal_mesh_component.h"

#include "renderer.h"

SkeletalMeshComponent::SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh) :
    TargetSkeletalMesh{mesh}
{
    std::vector<std::string> animations = mesh->GetAnimationNames();
    AnimationName = animations.back();
    BoneTransforms.resize(mesh->GetNumBones(), glm::identity<glm::mat4>());
}

void SkeletalMeshComponent::UpdateAnimation(float deltaSeconds, const Transform& transform)
{
    AnimationTime += deltaSeconds;
    TargetSkeletalMesh->GetAnimationFrames(AnimationUpdateArgs{AnimationTime, AnimationName, BoneTransforms});
}

void SkeletalMeshComponent::Draw(const glm::mat4& worldTransform) const
{
    TargetSkeletalMesh->Draw(BoneTransforms, worldTransform);
}