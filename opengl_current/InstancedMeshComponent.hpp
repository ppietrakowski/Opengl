#pragma once

#include "InstancedMesh.hpp"
#include "ResourceManager.hpp"

struct InstancedMeshComponent
{
    std::shared_ptr<InstancedMesh> TargetInstancedMesh;

    InstancedMeshComponent(const std::shared_ptr<StaticMesh>& mesh, const std::shared_ptr<Material>& material) :
        TargetInstancedMesh{std::make_shared<InstancedMesh>(mesh, material)}
    {
    }

    void AddInstance(const Transform& transform) const
    {
        TargetInstancedMesh->AddInstance(transform, 0);
    }

    void RemoveInstance(int index) const
    {
        TargetInstancedMesh->RemoveInstance(index);
    }

    void Draw(const glm::mat4& transform) const;
};
