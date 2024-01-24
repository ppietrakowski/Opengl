#pragma once

#include "skeletal_mesh.h"
#include "transform.h"

struct SkeletalMeshComponent
{
    // Calculated bone transforms, send to shader during draw.
    // Cannot use there UniformBuffer because animations are updated in seperate thread
    std::vector<glm::mat4> BoneTransforms;
    std::string AnimationName{DefaultAnimationName};
    std::shared_ptr<SkeletalMesh> TargetSkeletalMesh;
    float AnimationTime{0.0f};

    SkeletalMeshComponent() = default;
    SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh);

    void UpdateAnimation(float deltaSeconds, const Transform& transform);
    void Draw(const glm::mat4& worldTransform) const;
};



