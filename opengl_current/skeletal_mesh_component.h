#pragma once

#include "skeletal_mesh.h"
#include "transform.h"

struct SkeletalMeshComponent
{
    std::vector<glm::mat4> bone_transforms;
    std::string animation_name{kDefaultAnimationName};
    std::shared_ptr<SkeletalMesh> skeletal_mesh;
    float animation_time{0.0f};

    SkeletalMeshComponent() = default;
    SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh);

    void UpdateAnimation(float delta_seconds, const Transform& transform);
    void Draw(const glm::mat4& world_transform) const;
};



