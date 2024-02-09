#pragma once

#include <glm/glm.hpp>

enum class LightType : int32_t
{
    Directional,
    Point,
    Spot
};

// light data packed just to upload to uniform buffer 
struct LightData
{
    glm::vec3 Position{0, 0, 0};
    float Align0;

    glm::vec3 Direction{0, 1, 0};
    float Align1;

    glm::vec3 Color{1, 1, 1};

    float DirectionLength{0.0f};
    LightType Type{LightType::Directional};
    float CutOff{0.0f};
    float Intensity{0.0f};
    float OuterCutOff{0.0f};
};