#include "skeletal_mesh_component.h"

#include "renderer.h"

SkeletalMeshComponent::SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh) :
    skeletal_mesh{mesh} {
    std::vector<std::string> animations = mesh->GetAnimationNames();
    animation_name = animations.back();
    bone_transforms.resize(mesh->GetNumBones(), glm::identity<glm::mat4>());

    debug_bbox_min = skeletal_mesh->GetBboxMin();
    debug_bbox_max = skeletal_mesh->GetBboxMax();
}

void SkeletalMeshComponent::Draw(const glm::mat4& world_transform) {
    if (should_draw_debug_bounds) {
        Renderer::DrawDebugBox(debug_bbox_min, debug_bbox_max, world_transform);
    }

    skeletal_mesh->Draw(bone_transforms, world_transform);
}