#pragma once

#include "skeletal_mesh.h"

struct SkeletalMeshComponent
{
    std::vector<glm::mat4> BoneTransforms;
    std::string AnimationName{DefaultAnimationName};
    std::shared_ptr<SkeletalMesh> UsedSkeletalMesh;
    float AnimationTime{0.0f};

    SkeletalMeshComponent() = default;
    SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh);

    void UpdateAnimation(float deltaSeconds);
    void Draw(const glm::mat4& worldTransform);
};



