#pragma once

#include "InstancedMesh.hpp"
#include "ResourceManager.hpp"
#include "Datapack.hpp"
#include <vector>

struct InstancedMeshComponent
{
    std::shared_ptr<InstancedMesh> TargetInstancedMesh;
    std::vector<Transform> Transforms;

    InstancedMeshComponent(const std::shared_ptr<StaticMesh>& mesh, const std::shared_ptr<Material>& material) :
        TargetInstancedMesh{std::make_shared<InstancedMesh>(mesh, material)}
    {
    }

    void AddInstance(const Transform& transform)
    {
        Transforms.push_back(transform);
        TargetInstancedMesh->AddInstance(transform, 0);
    }

    void RemoveInstance(int index)
    {
        TargetInstancedMesh->RemoveInstance(index);
        Transforms.erase(Transforms.begin() + index);
    }

    void Draw(const glm::mat4& transform) const;
    Datapack Archived() const;
};
