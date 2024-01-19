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
        Renderer::SubmitMeshInstanced(SubmitCommandArgs{material_.get(), 0, static_mesh_->vertex_array_.get(), transform}, 
            *transform_buffer.uniform_buffer, transform_buffer.num_transforms_occupied);
    }
}

int InstancedMesh::AddInstance(const Transform& transform, int texture_id)
{
    auto it = transform_buffers_.begin();
    int id = num_instances_;

    bool recycled_indices = !free_instance_indices_.empty();
    if (recycled_indices)
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
        it = transform_buffers_.end() - 1;
    }

    if (recycled_indices)
    {
        it->UpdateTransform(transform.CalculateTransformMatrix(), id);
    }
    else
    {
        it->AddTransform(transform.CalculateTransformMatrix());
    }

    return num_instances_++;
}

void InstancedMesh::RemoveInstance(int index)
{
    num_instances_--;
    ASSERT(num_instances_ >= 0);
    Transform transform{};
    
    // easier than moving elements in memory
    transform.scale = glm::vec3(0, 0, 0);
    free_instance_indices_.emplace_back(index);

    UpdateInstance(index, transform);
}

void InstancedMesh::UpdateInstance(int index, const Transform& new_transform)
{
    auto it = transform_buffers_.begin();
    int id = index;
    
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
    for (InstancingTransformBuffer& transform_buffer : transform_buffers_)
    {
        transform_buffer.Clear();
    }

    num_instances_ = 0;
}
