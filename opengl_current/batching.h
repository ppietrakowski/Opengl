#pragma once

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

constexpr uint32_t kStartNumInstances = 16;

/* Just does bind and unbind within scope */
struct DebugVertexArrayScope
{
    VertexArray* target;

    DebugVertexArrayScope(VertexArray& target) :
        target{&target}
    {
        target.Bind();
    }

    ~DebugVertexArrayScope()
    {
        target->Unbind();
    }
};

template<typename>
struct BatchVertexCreator;

template <typename T>
struct BatchGeometryInfo
{
    std::span<const T> base_vertices;
    std::span<const uint32_t> base_indices;
    Transform transform;

    FORCE_INLINE int GetNumVertices() const
    {
        return static_cast<int>(base_vertices.size());
    }

    FORCE_INLINE glm::mat4 CalculateTransformMatrix() const
    {
        return transform.CalculateTransformMatrix();
    }
};

struct BatchElement
{
    int start_vertex;
    int end_vertex;

    int start_index;
    int end_index;
};

template <typename VerticesType>
class BatchBase
{
public:
    BatchBase(std::span<const VertexAttribute> attributes) :
        vertices_{1},
        indices_{1},
        attributes_{attributes.begin(), attributes.end()}
    {
        ResizeBuffers(kStartNumInstances * 64, kStartNumInstances * 64);
    }

    void ResizeBuffers(int max_num_vertices, int max_num_indices)
    {
        ASSERT(max_num_vertices >= 0);
        ASSERT(max_num_indices >= 0);

        // resize buffers instead just of reserve, because further on
        // vertices are added using setting value at index instead of emplace_back or push_back
        vertices_.resize(max_num_vertices);
        indices_.resize(max_num_indices);

        ELOG_VERBOSE(LOG_RENDERER, "Rebuilding buffers for instancing (vertex buffer size=%u vertices, max num indices=%u)", vertices_.capacity() * sizeof(VerticesType), indices_.capacity());

        // rebuild vertex and index buffers to match new size
        vertex_array_ = std::make_shared<VertexArray>();
        vertex_array_->AddVertexBuffer(std::make_shared<VertexBuffer>(static_cast<int>(vertices_.capacity() * sizeof(VerticesType))), attributes_);
        vertex_array_->SetIndexBuffer(std::make_shared<IndexBuffer>(static_cast<int>(indices_.capacity())));
        buffers_dirty_ = true;
    }

    void DrawTriangles(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bind_array_scope{*vertex_array_};
        UpdateBuffers();

        Renderer::SubmitTriangles(SubmitCommandArgs{&material, current_index_, vertex_array_.get(), transform});

        if (clear_post_draw_)
        {
            Clear();
        }
    }

    void DrawLines(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bind_array_scope{*vertex_array_};
        UpdateBuffers();

        Renderer::SubmitLines(SubmitCommandArgs{&material, current_index_, vertex_array_.get(), transform});

        if (clear_post_draw_)
        {
            Clear();
        }
    }

    void DrawPoints(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bind_array_scope{*vertex_array_};
        UpdateBuffers();

        Renderer::SubmitPoints(SubmitCommandArgs{&material, current_index_, vertex_array_.get(), transform});

        if (clear_post_draw_)
        {
            Clear();
        }
    }

    void Clear()
    {
        // just reset indices start index and num indices
        indices_start_index_ = 0;
        current_index_ = 0;
    }

    void RemoveInstance(int start_vertex, int end_vertex)
    {
        for (int i = start_vertex; i < end_vertex; ++i)
        {
            vertices_[i] = VerticesType{};
        }

        buffers_dirty_ = true;
    }

    template <typename ...Args>
    BatchElement QueueDraw(const BatchGeometryInfo<VerticesType>& geometry_info, Args&& ...args)
    {
        if (ShouldExpand(geometry_info))
        {
            Expand(geometry_info);
        }

        BatchElement instance{};
        instance.start_index = current_index_;
        instance.start_vertex = indices_start_index_;

        UpdateIndices(geometry_info);
        UpdateVertices(geometry_info, std::forward<Args>(args)...);

        instance.end_index = current_index_;
        instance.end_vertex = indices_start_index_;
        buffers_dirty_ = true;

        return instance;
    }

    template <typename ...Args>
    void UpdateInstance(int vertex_index, const VerticesType& vertex, const Transform& world_transform, Args&& ...args)
    {
        vertices_[vertex_index] = BatchVertexCreator<VerticesType>::CreateInstanceFrom(vertex, world_transform.CalculateTransformMatrix(), std::forward<Args>(args)...);
        buffers_dirty_ = true;
    }

    void EnableClearPostDraw()
    {
        clear_post_draw_ = true;
    }

    void DisableClearPostDraw()
    {
        clear_post_draw_ = false;
    }

private:
    std::vector<VerticesType> vertices_;
    std::vector<std::uint32_t> indices_;
    std::shared_ptr<VertexArray> vertex_array_;
    int indices_start_index_{0};
    int current_index_{0};
    std::vector<VertexAttribute> attributes_;

    // if not set, the clear method is not invoked automaticely
    bool clear_post_draw_ : 1{true};

    // flag used for check is need to update buffers
    bool buffers_dirty_ : 1{false};

private:
    void UpdateBuffers()
    {
        if (buffers_dirty_)
        {
            std::shared_ptr<VertexBuffer> buffer = vertex_array_->GetVertexBufferAt(0);
            std::shared_ptr<IndexBuffer> index_buffer = vertex_array_->GetIndexBuffer();

            buffer->UpdateVertices(vertices_.data(), indices_start_index_ * sizeof(VerticesType));
            index_buffer->UpdateIndices(indices_.data(), current_index_);
            buffers_dirty_ = false;
        }
    }

    bool ShouldExpand(const BatchGeometryInfo<VerticesType>& geometry_info) const
    {
        return (indices_start_index_ + geometry_info.GetNumVertices() >= vertices_.capacity()) ||
            (current_index_ + geometry_info.base_indices.size() >= indices_.capacity());
    }

    void Expand(const BatchGeometryInfo<VerticesType>& geometry_info)
    {
        int new_vertices_size = static_cast<int>(vertices_.capacity() + vertices_.capacity() / 2);
        int new_indices_size = static_cast<int>(indices_.capacity() + indices_.capacity() / 2);

        bool has_enough_vertices_capacity = new_vertices_size >= vertices_.size() + geometry_info.GetNumVertices();

        if (!has_enough_vertices_capacity)
        {
            new_vertices_size = static_cast<int>(vertices_.size() + geometry_info.GetNumVertices() + 1);
        }

        bool has_enough_indices_capacity = new_indices_size >= indices_.size() + geometry_info.base_indices.size();

        if (!has_enough_indices_capacity)
        {
            new_indices_size = static_cast<int>(indices_.size() + geometry_info.base_indices.size() + 1);
        }

        ResizeBuffers(new_vertices_size, new_indices_size);
    }

    void UpdateIndices(const BatchGeometryInfo<VerticesType>& geometry_info)
    {
        for (uint32_t index : geometry_info.base_indices)
        {
            // set though indexing is faster than emplace_back so use it
            indices_[current_index_++] = index + indices_start_index_;
        }
    }

    template <typename ...Args>
    void UpdateVertices(const BatchGeometryInfo<VerticesType>& geometry_info, Args&& ...args)
    {
        glm::mat4 transform = geometry_info.CalculateTransformMatrix();
        for (const VerticesType& vertex : geometry_info.base_vertices)
        {
            vertices_[indices_start_index_++] = BatchVertexCreator<VerticesType>::CreateInstanceFrom(vertex, transform, std::forward<Args>(args)...);
        }
    }
};