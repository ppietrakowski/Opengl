#pragma once

#include "lights.h"

struct DirectionalLightComponent {
    glm::vec3 direction{0, 1, 0};
    glm::vec3 color{1, 1, 1};
};

struct PointLightComponent {
    glm::vec3 direction{0, 1, 0};
    float direction_length{0.0f};

    glm::vec3 color{1, 1, 1};
};