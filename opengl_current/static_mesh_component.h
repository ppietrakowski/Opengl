#pragma once

#include "static_mesh.h"

struct StaticMeshComponent {
    std::shared_ptr<StaticMesh> static_mesh;

    StaticMeshComponent() = default;
    StaticMeshComponent(const std::shared_ptr<StaticMesh>& mesh);

    void Draw(const glm::mat4& world_transform) const;
};

