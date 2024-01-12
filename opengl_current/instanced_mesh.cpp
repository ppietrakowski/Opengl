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
    m_InstanceDraw.DisableClearPostDraw();
}

void InstancedMesh::Draw(const glm::mat4& transform, Material& material)
{
    m_InstanceDraw.DrawTriangles(transform, material);
}

std::int32_t InstancedMesh::AddInstance(const Transform& transform, std::int32_t textureId)
{
    m_InstanceDraw.QueueDraw(InstanceInfo<StaticMeshVertex>{m_BaseVertices, m_BaseIndices, transform}, textureId);
    return m_NumInstances++;
}

void InstancedMesh::RemoveInstance(std::int32_t index)
{
    std::int32_t startVertexIndex = static_cast<std::int32_t>(index * m_BaseVertices.size());
    std::int32_t endVertexIndex = static_cast<std::int32_t>(index * m_BaseVertices.size() + m_BaseVertices.size());
    m_InstanceDraw.RemoveInstance(startVertexIndex, endVertexIndex);
}
