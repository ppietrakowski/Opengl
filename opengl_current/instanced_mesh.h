#pragma once

#include "static_mesh.h"
#include "instancing.h"

#include <glm/gtc/quaternion.hpp>

class InstancedMesh {
public:
    InstancedMesh(const std::shared_ptr<StaticMesh>& staticMesh);

    void Draw(const glm::mat4& transform, Material& material);

    void QueueDraw(const Transform& transform, std::int32_t textureId);

    const StaticMesh& GetMesh() const {
        return *m_StaticMesh;
    }
private: 
    std::vector<StaticMeshVertex> m_BaseVertices;
    std::vector<std::uint32_t> m_BaseIndices;
    InstanceBase<StaticMeshVertex> m_InstanceDraw;
    std::shared_ptr<StaticMesh> m_StaticMesh;
};

