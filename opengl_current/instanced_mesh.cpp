#include "instanced_mesh.h"
#include "logging.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

InstancedMesh::InstancedMesh(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Material>& material) :
    m_StaticMesh{staticMesh},
    m_Material{material}
{
}

void InstancedMesh::Draw(const glm::mat4& transform)
{
    for (InstancingTransformBuffer& transformBuffer : m_TransformBuffers)
    {
        Shader& shader = m_Material->GetShader();

        shader.BindUniformBuffer(0, *transformBuffer.Buffer);

        Renderer::SubmitMeshInstanced(SubmitCommandArgs{m_Material.get(), 0, m_StaticMesh->m_VertexArray.get(), transform},
            *transformBuffer.Buffer, transformBuffer.NumTransformsOccupied);
    }
}

int InstancedMesh::AddInstance(const Transform& transform, int textureId)
{
    auto it = m_TransformBuffers.begin();
    int id = m_NumInstances;

    bool recycled_indices = !m_RecyclingMeshIndices.empty();
    if (recycled_indices)
    {
        id = m_RecyclingMeshIndices.back();
        m_RecyclingMeshIndices.pop_back();
    }

    // find relative index and coresponding uniform buffer
    while (id >= NumInstancesTransform)
    {
        id -= NumInstancesTransform;
        ++it;
    }

    bool bBufferPresent = it != m_TransformBuffers.end();

    if (!bBufferPresent)
    {
        m_TransformBuffers.emplace_back();

        // update iterator to point to new buffer
        it = m_TransformBuffers.end() - 1;
    }

    if (recycled_indices)
    {
        it->UpdateTransform(transform.CalculateTransformMatrix(), id);
    }
    else
    {
        it->AddTransform(transform.CalculateTransformMatrix());
    }

    return m_NumInstances++;
}

void InstancedMesh::RemoveInstance(int index)
{
    m_NumInstances--;
    ASSERT(m_NumInstances >= 0);
    Transform transform{};

    // easier than moving elements in memory
    transform.Scale = glm::vec3(0, 0, 0);
    m_RecyclingMeshIndices.emplace_back(index);

    UpdateInstance(index, transform);
}

void InstancedMesh::UpdateInstance(int index, const Transform& newTransform)
{
    auto it = m_TransformBuffers.begin();
    int id = index;

    // find relative index and coresponding uniform buffer
    while (id >= NumInstancesTransform)
    {
        id -= NumInstancesTransform;
        ++it;
    }

    if (it == m_TransformBuffers.end())
    {
        return;
    }

    it->UpdateTransform(newTransform.CalculateTransformMatrix(), id);
}

void InstancedMesh::Clear()
{
    for (InstancingTransformBuffer& transformBuffer : m_TransformBuffers)
    {
        transformBuffer.Clear();
    }

    m_NumInstances = 0;
}
