#include "InstancedMesh.hpp"
#include "Logging.hpp"
#include "Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

InstancedMesh::InstancedMesh(std::shared_ptr<StaticMesh> staticMesh, std::shared_ptr<Material> material) :
    m_StaticMesh{staticMesh},
    m_Material{material}
{
}

void InstancedMesh::Draw(const glm::mat4& transform)
{
    std::shared_ptr<Shader> shader = m_Material->GetShader();

    for (InstancingTransformBuffer& transformBuffer : m_TransformBuffers)
    {
        Renderer::SubmitMeshInstanced(m_StaticMesh->GetStaticMeshEntry(m_Lod), *m_Material, *transformBuffer.Buffer, transformBuffer.NumTransformsOccupied, transform);
    }
}

int InstancedMesh::AddInstance(const Transform& transform, int32_t textureId)
{
    auto it = m_TransformBuffers.begin();
    int32_t id = m_NumInstances;

    bool bShouldRecycleTransform = !m_RecyclingMeshIndices.empty();
    if (bShouldRecycleTransform)
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

    if (bShouldRecycleTransform)
    {
        it->UpdateTransform(transform.CalculateTransformMatrix(), id);
        return id;
    }
    else
    {
        it->AddTransform(transform.CalculateTransformMatrix());
    }

    return m_NumInstances++;
}

void InstancedMesh::RemoveInstance(int32_t index)
{
    m_NumInstances--;
    ASSERT(m_NumInstances >= 0);
    Transform transform{};

    // easier than moving elements in memory
    transform.Scale = glm::vec3(0, 0, 0);
    m_RecyclingMeshIndices.emplace_back(index);

    UpdateInstance(index, transform);
}

void InstancedMesh::UpdateInstance(int32_t index, const Transform& newTransform)
{
    auto it = m_TransformBuffers.begin();
    int32_t id = index;

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
