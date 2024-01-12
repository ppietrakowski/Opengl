#pragma once

#include "static_mesh.h"
#include "instancing.h"

#include <glm/gtc/quaternion.hpp>

class InstancedMesh
{
public:
    InstancedMesh(const std::shared_ptr<StaticMesh>& staticMesh);

    void Draw(const glm::mat4& transform, Material& material);

    // Adds new mesh instance. Returns index of newly created instance
    std::int32_t AddInstance(const Transform& transform, std::int32_t textureId);

    const StaticMesh& GetMesh() const
    {
        return *m_StaticMesh;
    }

    void RemoveInstance(std::int32_t index);

    std::int32_t GetSize() const
    {
        return m_NumInstances;
    }

private:
    std::vector<StaticMeshVertex> m_BaseVertices;
    std::vector<std::uint32_t> m_BaseIndices;
    InstanceBase<StaticMeshVertex> m_InstanceDraw;
    std::shared_ptr<StaticMesh> m_StaticMesh;
    std::int32_t m_NumInstances{0};
};

