#include "instanced_mesh.h"
#include "logging.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

InstancedMesh::InstancedMesh(const std::shared_ptr<StaticMesh>& staticMesh) :
    m_InstanceDraw(StaticMeshVertex::DataFormat),
    m_BaseVertices{staticMesh->Vertices},
    m_BaseIndices{staticMesh->Indices},
    m_StaticMesh{staticMesh}
{
}

void InstancedMesh::Draw(const glm::mat4& transform, Material& material)
{
    m_InstanceDraw.Draw(transform, material);
}

void InstancedMesh::QueueDraw(const Transform& transform, std::int32_t textureId)
{
    // prevent instancing something that's not visible
    if (!Renderer::IsVisibleToCamera(transform.Position, m_StaticMesh->GetBBoxMin(), m_StaticMesh->GetBBoxMax()))
    {
        return;
    }

    m_InstanceDraw.QueueDraw(InstanceInfo<StaticMeshVertex>{m_BaseVertices, m_BaseIndices, transform}, textureId);
}
