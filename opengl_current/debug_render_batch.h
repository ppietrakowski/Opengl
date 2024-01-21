#pragma once

#include "vertex_array.h"
#include "shader.h"
#include "batching.h"
#include <glm/glm.hpp>


struct DebugVertex {
    glm::vec3 Position;

    // Packed color to minimize size of updated data
    RgbaColor Color;
};

template<>
struct BatchVertexCreator<DebugVertex> {
    static DebugVertex CreateInstanceFrom(const DebugVertex& vertex, const glm::mat4& transform) {
        DebugVertex v{vertex};
        v.Position = transform * glm::vec4{v.Position,1};
        return v;
    }
};

class DebugRenderBatch {
public:
    DebugRenderBatch();

    void FlushDraw(Material& material);
    void AddBoxInstance(glm::vec3 box_min, glm::vec3 box_max, const Transform& transform, const glm::vec4& color);

    void AddLineInstance(const glm::vec3& start_pos, const glm::vec3& end_pos, const Transform& transform, const glm::vec4& color);

private:
    BatchBase<DebugVertex> batch_base_;
};

