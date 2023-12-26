#pragma once

#include "buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include <glm/glm.hpp>

class DebugRenderBatch {
public:
    DebugRenderBatch();

    void UploadBatchedData();
    void FlushDraw(Shader& shader);

    void AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform);

    bool CanBatchAnotherMesh(uint32_t num_indices) const;
    bool HasBatchedAnyPrimitive() const;

private:
    Buffer<glm::vec3> vertices;
    Buffer<uint32_t> indices;
    VertexArray vertex_array;
    uint32_t last_index_number{ 0 };
};

