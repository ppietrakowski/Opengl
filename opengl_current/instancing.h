#pragma once

#include "buffer.h"
#include "vertex_array.h"
#include "renderer.h"
#include "error_macros.h"
#include "logging.h"

#include "transform.h"

#include <vector>
#include <memory>
#include <span>
#include <execution>
#include <algorithm>

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
struct InstanceCreator;

template <typename T>
struct InstanceInfo {
    std::span<const T> base_vertices;
    std::span<const std::uint32_t> base_indices;
    Transform transform;

    FORCE_INLINE std::int32_t GetNumVertices() const {
        return static_cast<std::int32_t>(base_vertices.size());
    }

    FORCE_INLINE glm::mat4 CalculateTransformMatrix() const {
        return transform.CalculateTransformMatrix();
    }
};

template <typename VerticesType>
class InstanceBase {
public:
    InstanceBase(std::span<const VertexAttribute> attributes) :
        vertices_{1},
        indices_{1},
        attributes_{attributes.begin(), attributes.end()} {
        ResizeBuffers(kStartNumInstances * 64, kStartNumInstances * 64);
    }

    void ResizeBuffers(std::int32_t max_num_vertices, std::int32_t max_num_indices) {
        ASSERT(max_num_vertices >= 0);
        ASSERT(max_num_indices >= 0);

        vertices_.resize(max_num_vertices);
        indices_.resize(max_num_indices);

        ELOG_VERBOSE(LOG_RENDERER, "Rebuilding buffers for instancing (vertex buffer size=%u vertices, max num indices=%u)", vertices_.capacity() * sizeof(VerticesType), indices_.capacity());
        vertex_array_ = VertexArray::Create();
        vertex_array_->AddDynamicVertexBuffer(static_cast<std::int32_t>(vertices_.capacity() * sizeof(VerticesType)), attributes_);
        vertex_array_->SetIndexBuffer(IndexBuffer::CreateEmpty(static_cast<std::int32_t>(indices_.capacity())));
    }

    void Draw(const glm::mat4& transform, Material& material, RenderPrimitive primitive = RenderPrimitive::kTriangles) {

        DebugVertexArrayScope bind_array_scope{*vertex_array_};
        auto buffer = vertex_array_->GetVertexBufferAt(0);
        auto index_buffer = vertex_array_->GetIndexBuffer();

        buffer->UpdateVertices(vertices_.data(), start_index_ * sizeof(VerticesType));
        index_buffer->UpdateIndices(indices_.data(), current_index_);

        Renderer::Submit(material, current_index_, *vertex_array_, transform, primitive);
        start_index_ = 0;
        current_index_ = 0;
    }

    template <typename ...Args>
    void QueueDraw(const InstanceInfo<VerticesType>& instance_info, Args&& ...args) {
        if ((start_index_ + instance_info.GetNumVertices() >= vertices_.capacity()) ||
            (current_index_ + instance_info.base_indices.size() >= indices_.capacity())) {
            std::int32_t new_vertices_size = static_cast<std::int32_t>(vertices_.capacity() + vertices_.capacity() / 2);
            std::int32_t new_indices_size = static_cast<std::int32_t>(indices_.capacity() + indices_.capacity() / 2);

            if (new_vertices_size < vertices_.size() + instance_info.GetNumVertices()) {
                new_vertices_size = static_cast<std::int32_t>(vertices_.size() + instance_info.GetNumVertices() + 1);
            }

            if (new_indices_size < indices_.size() + instance_info.base_indices.size()) {
                new_indices_size = static_cast<std::int32_t>(indices_.size() + instance_info.base_indices.size() + 1);
            }

            ResizeBuffers(new_vertices_size, new_indices_size);
        }

        for (std::uint32_t index : instance_info.base_indices) {
            indices_[current_index_++] = index + start_index_;
        }

        glm::mat4 transform = instance_info.CalculateTransformMatrix();
        for (const VerticesType& vertex : instance_info.base_vertices) {
            vertices_[start_index_++] = InstanceCreator<VerticesType>::CreateInstanceFrom(vertex, transform, std::forward<Args>(args)...);
        }
    }

private:
    std::vector<VerticesType> vertices_;
    std::vector<std::uint32_t> indices_;
    std::shared_ptr<VertexArray> vertex_array_;
    std::int32_t start_index_{0};
    std::int32_t current_index_{0};

    std::vector<VertexAttribute> attributes_;
};