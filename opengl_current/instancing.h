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

constexpr std::uint32_t StartNumInstances = 16;

/* Just does bind and unbind within scope */
struct DebugVertexArrayScope
{
    VertexArray* Target;

    DebugVertexArrayScope(VertexArray& target) :
        Target{&target}
    {
        target.Bind();
    }

    ~DebugVertexArrayScope()
    {
        Target->Unbind();
    }
};

template<typename>
struct InstanceCreator;

template <typename T>
struct InstanceInfo
{
    std::span<const T> BaseVertices;
    std::span<const std::uint32_t> BaseIndices;
    Transform InstanceTransform;

    FORCE_INLINE std::int32_t GetNumVertices() const
    {
        return static_cast<std::int32_t>(BaseVertices.size());
    }

    FORCE_INLINE glm::mat4 CalculateTransformMatrix() const
    {
        return InstanceTransform.CalculateTransformMatrix();
    }
};

struct Instance
{
    std::int32_t StartVertex;
    std::int32_t EndVertex;

    std::int32_t StartIndex;
    std::int32_t EndIndex;
};

template <typename VerticesType>
class InstanceBase
{
public:
    InstanceBase(std::span<const VertexAttribute> attributes) :
        m_Vertices{1},
        m_Indices{1},
        m_Attribute{attributes.begin(), attributes.end()}
    {
        ResizeBuffers(StartNumInstances * 64, StartNumInstances * 64);
    }

    void ResizeBuffers(std::int32_t maxNumVertices, std::int32_t maxNumIndices)
    {
        ASSERT(maxNumVertices >= 0);
        ASSERT(maxNumIndices >= 0);

        // resize buffers instead just of reserve, because further on
        // vertices are added using setting value at index instead of emplace_back or push_back
        m_Vertices.resize(maxNumVertices);
        m_Indices.resize(maxNumIndices);

        ELOG_VERBOSE(LOG_RENDERER, "Rebuilding buffers for instancing (vertex buffer size=%u vertices, max num indices=%u)", m_Vertices.capacity() * sizeof(VerticesType), m_Indices.capacity());

        // rebuild vertex and index buffers to match new size
        m_VertexArray = std::make_shared<VertexArray>();
        m_VertexArray->AddVertexBuffer(std::make_shared<VertexBuffer>(static_cast<std::int32_t>(m_Vertices.capacity() * sizeof(VerticesType))), m_Attribute);
        m_VertexArray->SetIndexBuffer(std::make_shared<IndexBuffer>(static_cast<std::int32_t>(m_Indices.capacity())));
        m_bBuffersDirty = true;
    }

    void DrawTriangles(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bind_array_scope{*m_VertexArray};
        UpdateBuffers();

        Renderer::SubmitTriangles(material, m_CurrentIndex, *m_VertexArray, transform);

        if (m_bClearPostDraw)
        {
            Clear();
        }
    }

    void DrawLines(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bind_array_scope{*m_VertexArray};
        UpdateBuffers();

        Renderer::SubmitLines(material, m_CurrentIndex, *m_VertexArray, transform);

        if (m_bClearPostDraw)
        {
            Clear();
        }
    }

    void DrawPoints(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bind_array_scope{*m_VertexArray};
        UpdateBuffers();

        Renderer::SubmitPoints(material, m_CurrentIndex, *m_VertexArray, transform);

        if (m_bClearPostDraw)
        {
            Clear();
        }
    }

    void Clear()
    {
        // just reset indices start index and num indices
        m_IndicesStartIndex = 0;
        m_CurrentIndex = 0;
    }

    void RemoveInstance(std::int32_t startVertex, std::int32_t endVertex)
    {
        for (std::int32_t i = startVertex; i < endVertex; ++i)
        {
            m_Vertices[i] = VerticesType{};
        }

        m_bBuffersDirty = true;
    }

    template <typename ...Args>
    Instance QueueDraw(const InstanceInfo<VerticesType>& instanceInfo, Args&& ...args)
    {
        if (ShouldExpand(instanceInfo))
        {
            Expand(instanceInfo);
        }

        Instance instance{};
        instance.StartIndex = m_CurrentIndex;
        instance.StartVertex = m_IndicesStartIndex;

        UpdateIndices(instanceInfo);
        UpdateVertices(instanceInfo, std::forward<Args>(args)...);

        instance.EndIndex = m_CurrentIndex;
        instance.EndVertex = m_IndicesStartIndex;
        m_bBuffersDirty = true;

        return instance;
    }

    template <typename ...Args>
    void UpdateInstance(std::int32_t vertexIndex, const VerticesType& vertex, const Transform& worldTransform, Args&& ...args)
    {
        m_Vertices[vertexIndex] = InstanceCreator<VerticesType>::CreateInstanceFrom(vertex, worldTransform.CalculateTransformMatrix(), std::forward<Args>(args)...);
        m_bBuffersDirty = true;
    }

    void EnableClearPostDraw()
    {
        m_bClearPostDraw = true;
    }

    void DisableClearPostDraw()
    {
        m_bClearPostDraw = false;
    }

private:
    std::vector<VerticesType> m_Vertices;
    std::vector<std::uint32_t> m_Indices;
    std::shared_ptr<VertexArray> m_VertexArray;
    std::int32_t m_IndicesStartIndex{0};
    std::int32_t m_CurrentIndex{0};
    std::vector<VertexAttribute> m_Attribute;

    // if not set, the clear method is not invoked automaticely
    bool m_bClearPostDraw : 1{true};

    // flag used for check is need to update buffers
    bool m_bBuffersDirty : 1{false};

private:
    void UpdateBuffers()
    {
        if (m_bBuffersDirty)
        {
            std::shared_ptr<VertexBuffer> buffer = m_VertexArray->GetVertexBufferAt(0);
            std::shared_ptr<IndexBuffer> indexBuffer = m_VertexArray->GetIndexBuffer();

            buffer->UpdateVertices(m_Vertices.data(), m_IndicesStartIndex * sizeof(VerticesType));
            indexBuffer->UpdateIndices(m_Indices.data(), m_CurrentIndex);
            m_bBuffersDirty = false;
        }
    }

    bool ShouldExpand(const InstanceInfo<VerticesType>& instanceInfo) const
    {
        return (m_IndicesStartIndex + instanceInfo.GetNumVertices() >= m_Vertices.capacity()) ||
            (m_CurrentIndex + instanceInfo.BaseIndices.size() >= m_Indices.capacity());
    }

    void Expand(const InstanceInfo<VerticesType>& instanceInfo)
    {
        std::int32_t newVerticesSize = static_cast<std::int32_t>(m_Vertices.capacity() + m_Vertices.capacity() / 2);
        std::int32_t newIndicesSize = static_cast<std::int32_t>(m_Indices.capacity() + m_Indices.capacity() / 2);

        bool bHasEnoughVerticesCapacity = newVerticesSize >= m_Vertices.size() + instanceInfo.GetNumVertices();

        if (!bHasEnoughVerticesCapacity)
        {
            newVerticesSize = static_cast<std::int32_t>(m_Vertices.size() + instanceInfo.GetNumVertices() + 1);
        }

        bool bHasEnoughIndicesCapacity = newIndicesSize >= m_Indices.size() + instanceInfo.BaseIndices.size();

        if (!bHasEnoughIndicesCapacity)
        {
            newIndicesSize = static_cast<std::int32_t>(m_Indices.size() + instanceInfo.BaseIndices.size() + 1);
        }

        ResizeBuffers(newVerticesSize, newIndicesSize);
    }

    void UpdateIndices(const InstanceInfo<VerticesType>& instanceInfo)
    {
        for (std::uint32_t index : instanceInfo.BaseIndices)
        {
            // set though indexing is faster than emplace_back so use it
            m_Indices[m_CurrentIndex++] = index + m_IndicesStartIndex;
        }
    }

    template <typename ...Args>
    void UpdateVertices(const InstanceInfo<VerticesType>& instanceInfo, Args&& ...args)
    {
        glm::mat4 transform = instanceInfo.CalculateTransformMatrix();
        for (const VerticesType& vertex : instanceInfo.BaseVertices)
        {
            m_Vertices[m_IndicesStartIndex++] = InstanceCreator<VerticesType>::CreateInstanceFrom(vertex, transform, std::forward<Args>(args)...);
        }
    }
};