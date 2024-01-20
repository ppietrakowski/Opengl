#include "skeletal_mesh_component.h"

#include "renderer.h"

SkeletalMeshComponent::SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh) :
    skeletal_mesh{mesh} {
    std::vector<std::string> animations = mesh->GetAnimationNames();
    animation_name = animations.back();
    bone_transforms.resize(mesh->GetNumBones(), glm::identity<glm::mat4>());
}

void SkeletalMeshComponent::UpdateAnimation(float delta_seconds, const Transform& transform) {
    animation_time += delta_seconds;
    skeletal_mesh->GetAnimationFrames(AnimationUpdateArgs{animation_time, animation_name, bone_transforms});
}

void SkeletalMeshComponent::Draw(const glm::mat4& world_transform) const {
    skeletal_mesh->Draw(bone_transforms, world_transform);
}