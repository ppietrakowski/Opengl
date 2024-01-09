#pragma once

#include "skeletal_mesh.h"

struct SkeletalMeshComponent {
    std::vector<glm::mat4> bone_transforms;
    std::string animation_name;
    std::shared_ptr<SkeletalMesh> skeletal_mesh;
    float time{0.0f};
    bool should_draw_debug_bounds{false};
    glm::vec3 debug_bbox_min{0.0f, 0.0f, 0.0f};
    glm::vec3 debug_bbox_max{0.0f, 0.0f, 0.0f};

    SkeletalMeshComponent() = default;
    SkeletalMeshComponent(const std::shared_ptr<SkeletalMesh>& mesh);

    void UpdateAnimation(float delta_seconds);
    void Draw(const glm::mat4& world_transform);
};



