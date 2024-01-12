#pragma once

#include "instanced_mesh.h"
#include "resouce_manager.h"

struct InstancedMeshComponent
{
    std::shared_ptr<InstancedMesh> Instance;
    std::shared_ptr<Material> MainMaterial;

    InstancedMeshComponent(const std::shared_ptr<StaticMesh>& mesh) :
        Instance{std::make_shared<InstancedMesh>(mesh)},
        MainMaterial{mesh->MainMaterial}
    {
    }

    void AddInstance(const Transform& transform) const
    {
        Instance->AddInstance(transform, 0);
    }

    void RemoveInstance(std::int32_t index) const
    {
        Instance->RemoveInstance(index);
    }

    void Draw(const glm::mat4& transform) const;
};
