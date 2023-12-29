#pragma once

#include "static_mesh.h"

struct StaticMeshComponent
{
    std::shared_ptr<StaticMesh> Mesh;

    StaticMeshComponent() = default;
    StaticMeshComponent(const std::shared_ptr<StaticMesh>& mesh) :
        Mesh{mesh}
    {
    }

    void Draw(const glm::mat4& worldTransform) const
    {
        if (Mesh)
        {
            Mesh->Render(worldTransform);
        }
    }
};

