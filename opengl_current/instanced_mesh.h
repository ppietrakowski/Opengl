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

    void UpdateInstance(std::int32_t index, const Transform& newTransform)
    {
        std::int32_t startVertexIndex = static_cast<std::int32_t>(index * m_BaseVertices.size());
        std::int32_t endVertexIndex = static_cast<std::int32_t>(index * m_BaseVertices.size() + m_BaseVertices.size());

        for (std::int32_t i = startVertexIndex; i < endVertexIndex; ++i)
        {
            std::int32_t index = i - startVertexIndex;
            m_InstanceDraw.UpdateInstance(i, m_BaseVertices[index], newTransform, 0);
        }

        m_Transforms[index] = newTransform;
    }

    std::span<const Transform> GetTransforms() const
    {
        return m_Transforms;
    }

private:
    std::vector<StaticMeshVertex> m_BaseVertices;
    std::vector<std::uint32_t> m_BaseIndices;
    std::vector<Transform> m_Transforms;
    InstanceBase<StaticMeshVertex> m_InstanceDraw;
    std::shared_ptr<StaticMesh> m_StaticMesh;
    std::int32_t m_NumInstances{0};
};

