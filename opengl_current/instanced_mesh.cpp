#include "instanced_mesh.h"
#include "logging.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

void InstancedMesh::ResizeBuffers(std::int32_t maxVerticesSizeBytes, std::int32_t maxNumIndices) {
    ASSERT(maxVerticesSizeBytes >= 0);
    ASSERT(maxNumIndices >= 0);

    vertices_.Resize(maxVerticesSizeBytes);
    indices_.Resize(maxNumIndices);

    ELOG_VERBOSE(LOG_RENDERER, "Rebuilding buffers for instancing (vertex buffer size=%u vertices, max num indices=%u)", vertices_.GetCapacityBytes(), indices_.GetCapacity());
    vertex_array_ = VertexArray::Create();
    vertex_array_->AddDynamicBuffer(vertices_.GetCapacityBytes(), StaticMeshVertex::kDataFormat);
    vertex_array_->SetIndexBuffer(IndexBuffer::CreateEmpty(indices_.GetCapacity()));
}

constexpr std::uint32_t kStartNumInstances = 16;

InstancedMesh::InstancedMesh(const StaticMesh& static_mesh) :
    vertices_{1},
    indices_{1},
    base_indices_{static_mesh.indices},
    base_vertices_{static_mesh.vertices} {
    ResizeBuffers(kStartNumInstances * base_vertices_.size(), kStartNumInstances * base_indices_.size());
}


/* Just does bind and unbind within scope */
struct DebugVertexArrayScope {
    VertexArray* target;

    DebugVertexArrayScope(VertexArray& target) :
        target{&target} {
        target.Bind();
    }

    ~DebugVertexArrayScope() {
        target->Unbind();
    }
};

void InstancedMesh::Draw(const glm::mat4& transform, Material& material) {

    DebugVertexArrayScope bind_array_scope{*vertex_array_};
    auto buffer = vertex_array_->GetVertexBufferAt(0);
    auto index_buffer = vertex_array_->GetIndexBuffer();

    buffer->UpdateVertices(vertices_.GetRawData(), vertices_.GetSizeBytes());
    index_buffer->UpdateIndices(indices_.GetRawData(), indices_.GetSize());

    Renderer::Submit(material, indices_.GetSize(), *vertex_array_, transform);
    vertices_.ResetPtrToStart();
    indices_.ResetPtrToStart();
    start_index_ = 0;
}

void InstancedMesh::QueueDraw(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, std::int32_t texture_id) {
    if (vertices_.GetSize() + base_vertices_.size() > vertices_.GetCapacity()) {
        ResizeBuffers(vertices_.GetCapacity() + vertices_.GetCapacity() / 2,
            indices_.GetCapacity() + indices_.GetCapacity() / 2);
    }

    glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
    glm::mat3 normal_matrix = glm::inverseTranspose(transform);

    for (const StaticMeshVertex& vertex : base_vertices_) {
        StaticMeshVertex v(transform * glm::vec4{vertex.position, 1}, normal_matrix * vertex.normal, vertex.texture_coords);
        v.texture_id = texture_id;

        vertices_.AddInstance(v);
    }

    for (std::uint32_t index : base_indices_) {
        indices_.AddInstance(index + start_index_);
    }

    start_index_ += base_vertices_.size();
}
