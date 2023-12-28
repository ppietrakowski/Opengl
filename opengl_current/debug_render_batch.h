#pragma once

#include "buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include <glm/glm.hpp>

class DebugRenderBatch
{
public:
    DebugRenderBatch();

    void UploadBatchedData();
    void FlushDraw(IShader& shader);

    void AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform);

    bool CanBatchAnotherMesh(uint32_t numIndices) const;
    bool HasBatchedAnyPrimitive() const;

private:
    Buffer<glm::vec3> m_Vertices;
    Buffer<uint32_t> m_Indices;
    std::shared_ptr<IVertexArray> m_VertexArray;
    uint32_t m_LastIndexNumber{0};
};

