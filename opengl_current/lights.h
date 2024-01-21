#pragma once

#include <glm/glm.hpp>

enum class LightType : int32_t {
    Directional,
    Point,
    Spot
};

struct DirectionalLight {
    glm::vec3 position;
    glm::vec4 color;
};

struct PointLight {
    glm::vec3 position;
    glm::vec4 color;
};

// light data packed just to upload to uniform buffer 
struct LightData {
    glm::vec3 position{0, 0, 0};
    float pack0;
    
    glm::vec3 direction{0, 1, 0};
    float pack1;

    glm::vec3 color{1, 1, 1};

    float direction_length{0.0f};
    LightType light_type{LightType::Directional};
    float attentuation_factor{0.0f};
    float decreasing_start_angle{0.0f};
    float cut_off_angle{0.0f};
};