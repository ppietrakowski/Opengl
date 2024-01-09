#pragma once

#include "core.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

struct Transform {
    glm::vec3 position{0, 0, 0};
    glm::quat rotation{glm::vec3{0, 0, 0}};
    glm::vec3 scale{1, 1, 1};

    static Transform FromEulerAngles(const glm::vec3& euler_angles) {
        return Transform{glm::vec3{0,0,0}, glm::quat{glm::radians(euler_angles)}, glm::vec3{1, 1, 1}};
    }

    FORCE_INLINE glm::mat4 CalculateTransformMatrix() const {
        return glm::translate(glm::identity<glm::mat4>(), position)* glm::mat4_cast(rotation)* glm::scale(glm::identity<glm::mat4>(), scale);
    }
};