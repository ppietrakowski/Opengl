#pragma once

#include "vertex_array.h"
#include "shader.h"
#include "instancing.h"
#include <glm/glm.hpp>

struct DebugVertex
{
    glm::vec3 Position;
    glm::vec4 Color{0, 0, 0, 1};
};


template<>
struct InstanceCreator<DebugVertex>
{
    static DebugVertex CreateInstanceFrom(const DebugVertex& vertex, const glm::mat4& transform)
    {
        DebugVertex v{vertex};
        v.Position = transform * glm::vec4{v.Position,1};
        return v;
    }
};

class DebugRenderBatch
{
public:
    DebugRenderBatch();

    void FlushDraw(Material& material);
    void AddBoxInstance(glm::vec3 boxMin, glm::vec3 boxMax, const Transform& transform);
    void AddBoxInstance(glm::vec3 boxMin, glm::vec3 boxMax, const Transform& transform, const glm::vec4& color);

private:
    InstanceBase<DebugVertex> m_InstanceDraw;
};

