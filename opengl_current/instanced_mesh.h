#pragma once

#include "static_mesh.h"
#include "uniform_buffer.h"
#include "transform.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>


constexpr uint32_t NumInstancesTransform = 800;

// struct representing same transforms as there are in shader
struct InstancingTransforms
{
    glm::mat4 Transforms[NumInstancesTransform];
};

// Buffer for storing transform inside uniform buffer so it's faster to access
struct InstancingTransformBuffer
{
    std::shared_ptr<UniformBuffer> Buffer;
    int32_t NumTransformsOccupied{0};

    InstancingTransformBuffer() :
        Buffer(std::make_shared<UniformBuffer>(static_cast<int32_t>(sizeof(InstancingTransforms))))
    {
    }

    void AddTransform(const glm::mat4& transform)
    {
        Buffer->UpdateBuffer(glm::value_ptr(transform),
            sizeof(transform), NumTransformsOccupied * sizeof(transform));
        NumTransformsOccupied++;
    }

    void Clear()
    {
        NumTransformsOccupied = 0;
    }

    // Updates transform at relative index (from start of this buffer)
    void UpdateTransform(const glm::mat4& transform, int32_t relativeIndex) const
    {
        Buffer->UpdateBuffer(glm::value_ptr(transform), sizeof(transform), relativeIndex * sizeof(transform));
    }
};

class InstancedMesh
{
public:
    InstancedMesh(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Material>& material);

    void Draw(const glm::mat4& transform);

    // Adds new mesh instance. Returns index of newly created instance
    int32_t AddInstance(const Transform& transform, int32_t textureId);

    const StaticMesh& GetMesh() const
    {
        return *m_StaticMesh;
    }

    void RemoveInstance(int32_t index);

    int32_t GetSize() const
    {
        return m_NumInstances;
    }

    void UpdateInstance(int32_t index, const Transform& newTransform);

    void Clear();

    std::shared_ptr<Material> GetMaterial()
    {
        return m_Material;
    }

    void SetLod(int32_t lod)
    {
        m_Lod = lod;
    }

private:
    std::shared_ptr<StaticMesh> m_StaticMesh;
    int32_t m_NumInstances{0};
    std::shared_ptr<Material> m_Material;

    // transform buffers splitted into objects that can handle max 400 meshes
    std::vector<InstancingTransformBuffer> m_TransformBuffers;

    // used for recycling indices when removing instances
    std::vector<int32_t> m_RecyclingMeshIndices;

    int32_t m_Lod{0};
};

