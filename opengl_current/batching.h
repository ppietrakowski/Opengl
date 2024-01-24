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

constexpr uint32_t StartNumInstances = 16;

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
struct BatchVertexCreator;

template <typename T>
struct BatchGeometryInfo
{
    std::span<const T> BaseVertices;
    std::span<const uint32_t> BaseIndices;
    Transform GeometryTransform;

    FORCE_INLINE int GetNumVertices() const
    {
        return STD_ARRAY_NUM_ELEMENTS(BaseVertices);
    }

    FORCE_INLINE glm::mat4 CalculateTransformMatrix() const
    {
        return GeometryTransform.CalculateTransformMatrix();
    }
};

struct BatchElement
{
    int StartVertex;
    int EndVertex;

    int StartIndex;
    int EndIndex;
};

template <typename VerticesType>
class BatchBase
{
public:
    BatchBase(std::span<const VertexAttribute> attributes) :
        m_Vertices{1},
        m_Indices{1},
        m_Attributes{attributes.begin(), attributes.end()}
    {
        ResizeBuffers(StartNumInstances * 64, StartNumInstances * 64);
    }

    void ResizeBuffers(int maxNumVertices, int maxNumIndices)
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
        m_VertexArray->AddVertexBuffer(std::make_shared<VertexBuffer>(static_cast<int>(m_Vertices.capacity() * sizeof(VerticesType))), m_Attributes);
        m_VertexArray->SetIndexBuffer(std::make_shared<IndexBuffer>(static_cast<int>(m_Indices.capacity())));
        m_bBuffersDirty = true;
    }

    void DrawTriangles(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bindVertexArrayScope{*m_VertexArray};
        UpdateBuffers();

        Renderer::SubmitTriangles(SubmitCommandArgs{&material, m_CurrentIndex, m_VertexArray.get(), transform});

        if (m_bClearPostDraw)
        {
            Clear();
        }
    }

    void DrawLines(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bindVertexArrayScope{*m_VertexArray};
        UpdateBuffers();

        Renderer::SubmitLines(SubmitCommandArgs{&material, m_CurrentIndex, m_VertexArray.get(), transform});

        if (m_bClearPostDraw)
        {
            Clear();
        }
    }

    void DrawPoints(const glm::mat4& transform, Material& material)
    {
        DebugVertexArrayScope bindVertexArrayScope{*m_VertexArray};
        UpdateBuffers();

        Renderer::SubmitPoints(SubmitCommandArgs{&material, m_CurrentIndex, m_VertexArray.get(), transform});

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

    void RemoveInstance(int startVertex, int endVertex)
    {
        for (int i = startVertex; i < endVertex; ++i)
        {
            m_Vertices[i] = VerticesType{};
        }

        m_bBuffersDirty = true;
    }

    template <typename ...Args>
    BatchElement QueueDraw(const BatchGeometryInfo<VerticesType>& geometryInfo, Args&& ...args)
    {
        if (ShouldExpand(geometryInfo))
        {
            Expand(geometryInfo);
        }

        BatchElement instance{};
        instance.StartIndex = m_CurrentIndex;
        instance.StartVertex = m_IndicesStartIndex;

        UpdateIndices(geometryInfo);
        UpdateVertices(geometryInfo, std::forward<Args>(args)...);

        instance.EndVertex = m_CurrentIndex;
        instance.EndIndex = m_IndicesStartIndex;
        m_bBuffersDirty = true;

        return instance;
    }

    template <typename ...Args>
    void UpdateInstance(int vertexIndex, const VerticesType& vertex, const Transform& worldTransform, Args&& ...args)
    {
        m_Vertices[vertexIndex] = BatchVertexCreator<VerticesType>::CreateInstanceFrom(vertex, worldTransform.CalculateTransformMatrix(), std::forward<Args>(args)...);
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
    int m_IndicesStartIndex{0};
    int m_CurrentIndex{0};
    std::vector<VertexAttribute> m_Attributes;

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

    bool ShouldExpand(const BatchGeometryInfo<VerticesType>& geometryInfo) const
    {
        return (m_IndicesStartIndex + geometryInfo.GetNumVertices() >= m_Vertices.capacity()) ||
            (m_CurrentIndex + geometryInfo.BaseIndices.size() >= m_Indices.capacity());
    }

    void Expand(const BatchGeometryInfo<VerticesType>& geometryInfo)
    {
        int newVerticesSize = static_cast<int>(m_Vertices.capacity() + m_Vertices.capacity() / 2);
        int newIndicesSize = static_cast<int>(m_Indices.capacity() + m_Indices.capacity() / 2);

        bool bHasEnoughVerticesCapacity = newVerticesSize >= m_Vertices.size() + geometryInfo.GetNumVertices();

        if (!bHasEnoughVerticesCapacity)
        {
            newVerticesSize = static_cast<int>(m_Vertices.size() + geometryInfo.GetNumVertices() + 1);
        }

        bool bHasEnoughIndicesCapacity = newIndicesSize >= m_Indices.size() + geometryInfo.BaseIndices.size();

        if (!bHasEnoughIndicesCapacity)
        {
            newIndicesSize = static_cast<int>(m_Indices.size() + geometryInfo.BaseIndices.size() + 1);
        }

        ResizeBuffers(newVerticesSize, newIndicesSize);
    }

    void UpdateIndices(const BatchGeometryInfo<VerticesType>& geometryInfo)
    {
        for (uint32_t index : geometryInfo.BaseIndices)
        {
            // set though indexing is faster than emplace_back so use it
            m_Indices[m_CurrentIndex++] = index + m_IndicesStartIndex;
        }
    }

    template <typename ...Args>
    void UpdateVertices(const BatchGeometryInfo<VerticesType>& geometryInfo, Args&& ...args)
    {
        glm::mat4 transform = geometryInfo.CalculateTransformMatrix();
        for (const VerticesType& vertex : geometryInfo.BaseVertices)
        {
            m_Vertices[m_IndicesStartIndex++] = BatchVertexCreator<VerticesType>::CreateInstanceFrom(vertex, transform, std::forward<Args>(args)...);
        }
    }
};