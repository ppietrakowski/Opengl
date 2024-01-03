#pragma once

#include "skeletal_mesh.h"

struct SkeletalMeshComponent
{
    std::vector<glm::mat4> BoneTransforms;
    std::string AnimationName;
    std::shared_ptr<SkeletalMesh> Mesh;
    float Time{0.0f};
    bool bShouldDrawDebugBounds{false};
    glm::vec3 DebugBboxMin;
    glm::vec3 DebugBboxMax;

    SkeletalMeshComponent() = default;
    SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh);

    void UpdateAnimation(float deltaSeconds);
    void Draw(const glm::mat4& worldTransform);
};

inline void SkeletalMeshComponent::UpdateAnimation(float deltaSeconds)
{
    Time += deltaSeconds;
    Mesh->GetAnimationFrames(Time, AnimationName, BoneTransforms);

    DebugBboxMin = Mesh->GetBboxMin();
    DebugBboxMax = Mesh->GetBboxMax();
}


