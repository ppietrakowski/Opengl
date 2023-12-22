#pragma once

#include <glm/glm.hpp>

struct BoundingBox {
    glm::vec3 min_bound{ 0, 0, 0 };
    glm::vec3 max_bound{ 0, 0, 0 };
};

