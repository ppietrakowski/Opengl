#pragma once

#include "transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct TransformComponent {
    glm::vec3 position{0, 0, 0};
    glm::quat rotation{glm::vec3{0, 0, 0}};
    glm::vec3 scale{1, 1, 1};

    glm::mat4 GetWorldTransformMatrix() const;

    void Translate(const glm::vec3& pos);

    void SetEulerAngles(const glm::vec3& euler_angles);
    void SetEulerAngles(float pitch, float yaw, float roll);

    void AddEulerAngles(const glm::vec3& euler_angles);
    glm::vec3 GetEulerAngles() const;

    glm::vec3 GetForwardVector() const;
    glm::vec3 GetRightVector() const;
    glm::vec3 GetUpVector() const;

    Transform GetAsTransform() const;
};

FORCE_INLINE void TransformComponent::Translate(const glm::vec3& pos) {
    position += pos;
}

FORCE_INLINE void TransformComponent::SetEulerAngles(const glm::vec3& euler_angles) {
    rotation = glm::quat{glm::radians(euler_angles)};
}

FORCE_INLINE void TransformComponent::SetEulerAngles(float pitch, float yaw, float roll) {
    rotation = glm::quat{glm::radians(glm::vec3{pitch, yaw, roll})};
}

FORCE_INLINE void TransformComponent::AddEulerAngles(const glm::vec3& euler_angles) {
    rotation *= glm::quat{glm::vec3{euler_angles}};
}

FORCE_INLINE glm::mat4 TransformComponent::GetWorldTransformMatrix() const {
    return glm::translate(glm::identity<glm::mat4>(), position) *
        glm::mat4_cast(rotation) *
        glm::scale(glm::identity<glm::mat4>(), scale);
}

FORCE_INLINE glm::vec3 TransformComponent::GetEulerAngles() const {
    return glm::degrees(glm::eulerAngles(rotation));
}

FORCE_INLINE glm::vec3 TransformComponent::GetForwardVector() const {
    return glm::normalize(rotation * glm::vec3{0, 0, -1});
}

FORCE_INLINE glm::vec3 TransformComponent::GetRightVector() const {
    return glm::normalize(rotation * glm::vec3{1, 0, 0});
}

FORCE_INLINE glm::vec3 TransformComponent::GetUpVector() const {
    return glm::normalize(rotation * glm::vec3{0, 1, 0});
}

FORCE_INLINE Transform TransformComponent::GetAsTransform() const {
    return Transform{position, rotation, scale};
}
