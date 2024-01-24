#pragma once

#include "static_mesh.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "uniform_buffer.h"

constexpr uint32_t NumInstancesTransform = 400;

// struct representing same transforms as there are in shader
struct InstancingTransforms {
    glm::mat4 Transforms[NumInstancesTransform];
};

// Buffer for storing transform inside uniform buffer so it's faster to access
struct InstancingTransformBuffer {
    std::shared_ptr<UniformBuffer> Buffer;
    int NumTransformsOccupied{0};

    InstancingTransformBuffer() :
        Buffer(std::make_shared<UniformBuffer>(static_cast<int>(sizeof(InstancingTransforms)))) {
    }

    void AddTransform(const glm::mat4& transform) {
        Buffer->UpdateBuffer(glm::value_ptr(transform),
            sizeof(transform), NumTransformsOccupied * sizeof(transform));
        NumTransformsOccupied++;
    }

    void Clear() {
        NumTransformsOccupied = 0;
    }

    // Updates transform at relative index (from start of this buffer)
    void UpdateTransform(const glm::mat4& transform, int relativeIndex) const {
        Buffer->UpdateBuffer(glm::value_ptr(transform), sizeof(transform), relativeIndex * sizeof(transform));
    }
};

class InstancedMesh {
public:
    InstancedMesh(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Material>& material);

    void Draw(const glm::mat4& transform);

    // Adds new mesh instance. Returns index of newly created instance
    int AddInstance(const Transform& transform, int textureId);

    const StaticMesh& GetMesh() const {
        return *m_StaticMesh;
    }

    void RemoveInstance(int index);

    int GetSize() const {
        return m_NumInstances;
    }

    void UpdateInstance(int index, const Transform& newTransform);

    void Clear();

private:
    std::shared_ptr<StaticMesh> m_StaticMesh;
    int m_NumInstances{0};
    std::shared_ptr<Material> m_Material;

    // transform buffers splitted into objects that can handle max 400 meshes
    std::vector<InstancingTransformBuffer> m_TransformBuffers;

    // used for recycling indices when removing instances
    std::vector<int> m_RecyclingMeshIndices;
};

