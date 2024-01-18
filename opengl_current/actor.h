#pragma once

#include "transform.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct ActorTagComponent
{
    std::string name;
    std::string tag{"Default"};
};

struct TransformComponent
{
    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent& operator=(const TransformComponent&) = default;

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

class Level;

// Basic gameplay object. This class is copy constructible
class Actor
{
    friend class Level;
public:
    Actor();
    Actor(const Actor&) = default;
    Actor& operator=(const Actor&) = default;

    template <typename T>
    T& GetComponent()
    {
        return entity_handle_.get<T>();
    }

    template <typename T>
    const T& GetComponent() const
    {
        return entity_handle_.get<T>();
    }

    template <typename T, typename ...Args>
    void AddComponent(Args&& ...args)
    {
        entity_handle_.emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    void RemoveComponent()
    {
        entity_handle_.erase<T>();
    }

    const std::string& GetName() const;
    void SetName(const std::string& name);

    const Level* GetHomeLevel() const
    {
        return home_level_;
    }

    Level* GetHomeLevel()
    {
        return home_level_;
    }

    void DestroyActor();
    bool IsAlive() const;

    bool operator==(const Actor& other) const
    {
        return entity_handle_.entity() == other.entity_handle_.entity();
    }

    bool operator!=(const Actor& other) const
    {
        return entity_handle_.entity() != other.entity_handle_.entity();
    }

    const TransformComponent& GetTransform() const
    {
        return entity_handle_.get<TransformComponent>();
    }

    TransformComponent& GetTransform()
    {
        return entity_handle_.get<TransformComponent>();
    }

private:
    entt::handle entity_handle_;
    Level* home_level_{nullptr};
};

FORCE_INLINE void TransformComponent::Translate(const glm::vec3& pos)
{
    position += pos;
}

FORCE_INLINE void TransformComponent::SetEulerAngles(const glm::vec3& euler_angles)
{
    rotation = glm::quat{glm::radians(euler_angles)};
}

FORCE_INLINE void TransformComponent::SetEulerAngles(float pitch, float yaw, float roll)
{
    rotation = glm::quat{glm::radians(glm::vec3{pitch, yaw, roll})};
}

FORCE_INLINE void TransformComponent::AddEulerAngles(const glm::vec3& euler_angles)
{
    rotation *= glm::quat{glm::vec3{euler_angles}};
}

FORCE_INLINE glm::mat4 TransformComponent::GetWorldTransformMatrix() const
{
    return glm::translate(glm::identity<glm::mat4>(), position) *
        glm::mat4_cast(rotation) *
        glm::scale(glm::identity<glm::mat4>(), scale);
}

FORCE_INLINE glm::vec3 TransformComponent::GetEulerAngles() const
{
    return glm::degrees(glm::eulerAngles(rotation));
}

FORCE_INLINE glm::vec3 TransformComponent::GetForwardVector() const
{
    return glm::normalize(rotation * glm::vec3{0, 0, -1});
}

FORCE_INLINE glm::vec3 TransformComponent::GetRightVector() const
{
    return glm::normalize(rotation * glm::vec3{1, 0, 0});
}

FORCE_INLINE glm::vec3 TransformComponent::GetUpVector() const
{
    return glm::normalize(rotation * glm::vec3{0, 1, 0});
}

FORCE_INLINE Transform TransformComponent::GetAsTransform() const
{
    return Transform{position, rotation, scale};
}
