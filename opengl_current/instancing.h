#pragma once

#include "buffer.h"
#include "vertex_array.h"
#include "renderer.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <vector>
#include <memory>
#include <span>

constexpr std::uint32_t kStartNumInstances = 16;


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

template<typename>
struct TInstanceConvert;

template <typename T>
class InstanceBase {
public:
    InstanceBase(std::span<const VertexAttribute> attributes) :
        vertices_{1},
        indices_{1},
        attributes_{attributes.begin(), attributes.end()}
    {
        ResizeBuffers(kStartNumInstances * 64, kStartNumInstances * 64);
    }

    void ResizeBuffers(std::int32_t max_num_vertices, std::int32_t max_num_indices) {
        ASSERT(max_num_vertices >= 0);
        ASSERT(max_num_indices >= 0);

        vertices_.Resize(max_num_vertices);
        indices_.Resize(max_num_indices);

        ELOG_VERBOSE(LOG_RENDERER, "Rebuilding buffers for instancing (vertex buffer size=%u vertices, max num indices=%u)", vertices_.GetCapacityBytes(), indices_.GetCapacity());
        vertex_array_ = VertexArray::Create();
        vertex_array_->AddDynamicBuffer(vertices_.GetCapacityBytes(), attributes_);
        vertex_array_->SetIndexBuffer(IndexBuffer::CreateEmpty(indices_.GetCapacity()));
    }
    
    void Draw(const glm::mat4& transform, Material& material, RenderPrimitive primitive = RenderPrimitive::kTriangles) {

        DebugVertexArrayScope bind_array_scope{*vertex_array_};
        auto buffer = vertex_array_->GetVertexBufferAt(0);
        auto index_buffer = vertex_array_->GetIndexBuffer();

        buffer->UpdateVertices(vertices_.GetRawData(), vertices_.GetSizeBytes());
        index_buffer->UpdateIndices(indices_.GetRawData(), indices_.GetSize());

        Renderer::Submit(material, indices_.GetSize(), *vertex_array_, transform, primitive);
        vertices_.ResetPtrToStart();
        indices_.ResetPtrToStart();
        start_index_ = 0;
    }

    template <typename ...Args>
    void QueueDraw(std::span<const T> base_vertices, std::span<const std::uint32_t> base_indices, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, Args&& ...args) {
        if (vertices_.GetSize() + base_vertices.size() > vertices_.GetCapacity()) {
            ResizeBuffers(vertices_.GetCapacity() + vertices_.GetCapacity() / 2,
                indices_.GetCapacity() + indices_.GetCapacity() / 2);
        }

        glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
        glm::mat3 normal_matrix = glm::inverseTranspose(transform);

        for (const T& vertex : base_vertices) {
            T v = TInstanceConvert<T>::ConvertInstancePosToT(transform * glm::vec4{vertex.position, 1}, normal_matrix * vertex.normal, vertex.texture_coords, std::forward<Args>(args)...);
            vertices_.AddInstance(v);
        }

        for (std::uint32_t index : base_indices) {
            indices_.AddInstance(index + start_index_);
        }

        start_index_ += base_vertices.size();
    }

private:
    Buffer<T> vertices_;
    Buffer<std::uint32_t> indices_;
    std::shared_ptr<VertexArray> vertex_array_;
    std::int32_t start_index_{0};
    
    std::vector<VertexAttribute> attributes_;
};