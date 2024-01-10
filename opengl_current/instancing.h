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

        m_Vertices.resize(maxNumVertices);
        m_Indices.resize(maxNumIndices);

        ELOG_VERBOSE(LOG_RENDERER, "Rebuilding buffers for instancing (vertex buffer size=%u vertices, max num indices=%u)", m_Vertices.capacity() * sizeof(VerticesType), m_Indices.capacity());
        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddDynamicVertexBuffer(static_cast<std::int32_t>(m_Vertices.capacity() * sizeof(VerticesType)), m_Attribute);
        m_VertexArray->SetIndexBuffer(IndexBuffer::CreateEmpty(static_cast<std::int32_t>(m_Indices.capacity())));
    }

    void Draw(const glm::mat4& transform, Material& material, RenderPrimitive primitive = RenderPrimitive::Triangles)
    {

        DebugVertexArrayScope bind_array_scope{*m_VertexArray};
        auto buffer = m_VertexArray->GetVertexBufferAt(0);
        auto index_buffer = m_VertexArray->GetIndexBuffer();

        buffer->UpdateVertices(m_Vertices.data(), m_IndicesStartIndex * sizeof(VerticesType));
        index_buffer->UpdateIndices(m_Indices.data(), m_CurrentIndex);

        Renderer::Submit(material, m_CurrentIndex, *m_VertexArray, transform, primitive);
        m_IndicesStartIndex = 0;
        m_CurrentIndex = 0;
    }

    template <typename ...Args>
    void QueueDraw(const InstanceInfo<VerticesType>& instanceInfo, Args&& ...args)
    {
        if ((m_IndicesStartIndex + instanceInfo.GetNumVertices() >= m_Vertices.capacity()) ||
            (m_CurrentIndex + instanceInfo.BaseIndices.size() >= m_Indices.capacity()))
        {
            std::int32_t newVerticesSize = static_cast<std::int32_t>(m_Vertices.capacity() + m_Vertices.capacity() / 2);
            std::int32_t newIndicesSize = static_cast<std::int32_t>(m_Indices.capacity() + m_Indices.capacity() / 2);

            if (newVerticesSize < m_Vertices.size() + instanceInfo.GetNumVertices())
            {
                newVerticesSize = static_cast<std::int32_t>(m_Vertices.size() + instanceInfo.GetNumVertices() + 1);
            }

            if (newIndicesSize < m_Indices.size() + instanceInfo.BaseIndices.size())
            {
                newIndicesSize = static_cast<std::int32_t>(m_Indices.size() + instanceInfo.BaseIndices.size() + 1);
            }

            ResizeBuffers(newVerticesSize, newIndicesSize);
        }

        for (std::uint32_t index : instanceInfo.BaseIndices)
        {
            m_Indices[m_CurrentIndex++] = index + m_IndicesStartIndex;
        }

        glm::mat4 transform = instanceInfo.CalculateTransformMatrix();
        for (const VerticesType& vertex : instanceInfo.BaseVertices)
        {
            m_Vertices[m_IndicesStartIndex++] = InstanceCreator<VerticesType>::CreateInstanceFrom(vertex, transform, std::forward<Args>(args)...);
        }
    }

private:
    std::vector<VerticesType> m_Vertices;
    std::vector<std::uint32_t> m_Indices;
    std::shared_ptr<VertexArray> m_VertexArray;
    std::int32_t m_IndicesStartIndex{0};
    std::int32_t m_CurrentIndex{0};

    std::vector<VertexAttribute> m_Attribute;
};