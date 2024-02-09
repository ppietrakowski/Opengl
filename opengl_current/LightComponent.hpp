#pragma once

#include "Lights.hpp"

struct DirectionalLightComponent
{
    glm::vec3 Direction{0, 1, 0};
    glm::vec3 Color{1, 1, 1};
};

struct PointLightComponent
{
    float DirectionLength{0.0f};

    glm::vec3 Color{1, 1, 1};
    float Intensity{1.0f};
};

struct SpotLightComponent
{
    glm::vec3 Direction{0, 1, 0};
    float DirectionLength{0.0f};
    glm::vec3 Color{1, 1, 1};
    float CutOffAngle{0.0f};
    float Intensity{1.0f};
    float OuterCutOffAngle{1.0f};
};