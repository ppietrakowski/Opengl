#pragma once

#include "buffer.h"
#include "vertex_array.h"
#include "shader.h"
#include "instancing.h"
#include <glm/glm.hpp>

struct DebugVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture_coords;
};

template<>
struct TInstanceConvert<DebugVertex> {
    template <typename ...Args>
    static DebugVertex ConvertInstancePosToT(glm::vec3 v, glm::vec3 normal, glm::vec2 texture_coords, Args&& ...args) {
        return DebugVertex{v};
    }
};

class DebugRenderBatch {
public:
    DebugRenderBatch();

    void FlushDraw(Material& material);
    void AddBoxInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

private:
    InstanceBase<DebugVertex> instance_draw_;
};

