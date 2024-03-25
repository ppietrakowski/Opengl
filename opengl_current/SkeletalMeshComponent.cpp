#include "SkeletalMeshComponent.hpp"

#include "Renderer.hpp"

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
    Renderer::SubmitSkeleton(*TargetSkeletalMesh, worldTransform, BoneTransforms);
}

Datapack SkeletalMeshComponent::Archived() const
{
    Datapack p;
    p["AnimationName"] = AnimationName;
    p["SkeletalMesh"] = TargetSkeletalMesh->GetPath();
    p["AnimationTime"] = AnimationTime;
    return p;
}
