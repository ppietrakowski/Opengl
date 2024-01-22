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
    DebugRenderBatch(std::shared_ptr<Shader> shader);

    void FlushDraw();
    void AddBoxInstance(const Box& box, const Transform& transform, const glm::vec4& color);

    void AddLineInstance(const Line& line, const Transform& transform, const glm::vec4& color);

private:
    BatchBase<DebugVertex> batch_base_;
    std::shared_ptr<Material> material_;
};

