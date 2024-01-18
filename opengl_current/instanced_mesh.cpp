#include "instanced_mesh.h"
#include "logging.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

InstancedMesh::InstancedMesh(const std::shared_ptr<StaticMesh>& static_mesh, const std::shared_ptr<Material>& material) :
    static_mesh_{static_mesh},
    material_{material}
{
}

void InstancedMesh::Draw(const glm::mat4& transform)
{
    for (InstancingTransformBuffer& transform_buffer : transform_buffers_)
    {
        material_->GetShader().BindUniformBuffer(0, *transform_buffer.uniform_buffer);
        Renderer::SubmitMeshInstanced(InstancingSubmission{material_.get(), static_mesh_->vertex_array_.get(), transform_buffer.uniform_buffer.get(), transform_buffer.num_transforms_occupied, transform});
    }
}

std::int32_t InstancedMesh::AddInstance(const Transform& transform, std::int32_t textureId)
{
    auto it = transform_buffers_.begin();
    std::int32_t id = num_instances_;

    if (!free_instance_indices_.empty())
    {
        id = free_instance_indices_.back();
        free_instance_indices_.pop_back();
    }

    // find relative index and coresponding uniform buffer
    while (id >= kNumInstancesTransform)
    {
        id -= kNumInstancesTransform;
        ++it;
    }

    bool buffer_present = it != transform_buffers_.end();

    if (!buffer_present)
    {
        transform_buffers_.emplace_back();
        
        // update iterator to point to new buffer
        it = transform_buffers_.begin();
        std::advance(it, transform_buffers_.size() - 1);
    }

    it->AddTransform(transform.CalculateTransformMatrix());
    return num_instances_++;
}

void InstancedMesh::RemoveInstance(std::int32_t index)
{
    num_instances_--;
    ASSERT(num_instances_ >= 0);
    Transform transform{};
    
    // easier than moving elements in memory
    transform.scale = glm::vec3(0, 0, 0);
    free_instance_indices_.emplace_back(index);

    UpdateInstance(index, transform);
}

void InstancedMesh::UpdateInstance(std::int32_t index, const Transform& new_transform)
{
    auto it = transform_buffers_.begin();
    std::int32_t id = index;
    
    // find relative index and coresponding uniform buffer
    while (id >= kNumInstancesTransform)
    {
        id -= kNumInstancesTransform;
        ++it;
    }

    if (it == transform_buffers_.end())
    {
        return;
    }

    it->UpdateTransform(new_transform.CalculateTransformMatrix(), id);
}

void InstancedMesh::Clear()
{
    transform_buffers_.clear();
    num_instances_ = 0;
}
