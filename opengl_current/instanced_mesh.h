#pragma once

#include "static_mesh.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "uniform_buffer.h"

constexpr uint32_t kNumInstancesTransform = 400;

// struct representing same transforms as there are in shader
struct InstancingTransforms {
    glm::mat4 transforms[kNumInstancesTransform];
};

// Buffer for storing transform inside uniform buffer so it's faster to access
struct InstancingTransformBuffer {
    std::shared_ptr<UniformBuffer> uniform_buffer;
    int num_transforms_occupied{0};

    InstancingTransformBuffer() :
        uniform_buffer(std::make_shared<UniformBuffer>(static_cast<int>(sizeof(InstancingTransforms)))) {
    }

    void AddTransform(const glm::mat4& transform) {
        uniform_buffer->UpdateBuffer(glm::value_ptr(transform),
            sizeof(transform), num_transforms_occupied * sizeof(transform));
        num_transforms_occupied++;
    }

    void Clear() {
        num_transforms_occupied = 0;
    }

    // Updates transform at relative index (from start of this buffer)
    void UpdateTransform(const glm::mat4& transform, int relative_index) const {
        uniform_buffer->UpdateBuffer(glm::value_ptr(transform), sizeof(transform), relative_index * sizeof(transform));
    }
};

class InstancedMesh {
public:
    InstancedMesh(const std::shared_ptr<StaticMesh>& static_mesh, const std::shared_ptr<Material>& material);

    void Draw(const glm::mat4& transform);

    // Adds new mesh instance. Returns index of newly created instance
    int AddInstance(const Transform& transform, int texture_id);

    const StaticMesh& GetMesh() const {
        return *static_mesh_;
    }

    void RemoveInstance(int index);

    int GetSize() const {
        return num_instances_;
    }

    void UpdateInstance(int index, const Transform& new_transform);

    void Clear();

private:
    std::shared_ptr<StaticMesh> static_mesh_;
    int num_instances_{0};
    std::shared_ptr<Material> material_;

    std::shared_ptr<VertexArray> vertex_array_;

    // transform buffers splitted into objects that can handle max 400 meshes
    std::vector<InstancingTransformBuffer> transform_buffers_;

    // used for recycling indices when removing instances
    std::vector<int> free_instance_indices_;
};

