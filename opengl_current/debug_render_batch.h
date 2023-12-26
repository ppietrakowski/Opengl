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
    Buffer<glm::vec3> vertices_;
    Buffer<uint32_t> indices_;
    std::shared_ptr<VertexArray> vertex_array_;
    uint32_t last_index_number_{ 0 };
};

