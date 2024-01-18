#pragma once

#include "instanced_mesh.h"
#include "resouce_manager.h"

struct InstancedMeshComponent
{
    std::shared_ptr<InstancedMesh> instanced_mesh;

    InstancedMeshComponent(const std::shared_ptr<StaticMesh>& mesh, const std::shared_ptr<Material>& material) :
        instanced_mesh{std::make_shared<InstancedMesh>(mesh, material)}
    {
    }

    void AddInstance(const Transform& transform) const
    {
        instanced_mesh->AddInstance(transform, 0);
    }

    void RemoveInstance(std::int32_t index) const
    {
        instanced_mesh->RemoveInstance(index);
    }

    void Draw(const glm::mat4& transform) const;
};
