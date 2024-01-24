#pragma once

#include "core.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

struct Transform
{
    glm::vec3 Position{0, 0, 0};
    glm::quat Rotation{glm::vec3{0, 0, 0}};
    glm::vec3 Scale{1, 1, 1};

    static Transform FromEulerAngles(const glm::vec3& eulerAngles);

    glm::mat4 CalculateTransformMatrix() const;
};

FORCE_INLINE Transform Transform::FromEulerAngles(const glm::vec3& eulerAngles)
{
    return Transform{glm::vec3{0,0,0}, glm::quat{glm::radians(eulerAngles)}, glm::vec3{1, 1, 1}};
}

FORCE_INLINE glm::mat4 Transform::CalculateTransformMatrix() const
{
    return glm::translate(glm::identity<glm::mat4>(), Position) *
        glm::mat4_cast(Rotation) *
        glm::scale(glm::identity<glm::mat4>(), Scale);
}