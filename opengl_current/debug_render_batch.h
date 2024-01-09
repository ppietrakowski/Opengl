#pragma once

#include "buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include "instancing.h"
#include <glm/glm.hpp>

struct DebugVertex {
    glm::vec3 position;
    glm::vec4 color{0, 0, 0, 1};
};


template<>
struct InstanceCreator<DebugVertex> {
    static DebugVertex CreateInstanceFrom(const DebugVertex& vertex, const glm::mat4& transform) {
        DebugVertex v{vertex};
        v.position = transform * glm::vec4{v.position,1};
        return v;
    }
};

class DebugRenderBatch {
public:
    DebugRenderBatch();

    void FlushDraw(Material& material);
    void AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform);
    void AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const Transform& transform, const glm::vec4& color);

private:
    InstanceBase<DebugVertex> instance_draw_;
};

