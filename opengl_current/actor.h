#pragma once

#include "transform.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct ActorTagComponent
{
    std::string Name;
    std::string Tag{"Default"};
    bool bIsAlive{true};
};

struct TransformComponent
{
    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent& operator=(const TransformComponent&) = default;

    glm::vec3 Position{0, 0, 0};
    glm::quat Rotation{glm::vec3{0, 0, 0}};
    glm::vec3 Scale{1, 1, 1};

    glm::mat4 GetWorldTransformMatrix() const;
   
    void Translate(const glm::vec3& pos);

    void SetEulerAngles(const glm::vec3& eulerAngles);
    void SetEulerAngles(float pitch, float yaw, float roll);

    void AddEulerAngles(const glm::vec3& eulerAngles);
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
        return m_EntityHandle.get<T>();
    }

    template <typename T>
    const T& GetComponent() const
    {
        return m_EntityHandle.get<T>();
    }

    template <typename T, typename ...Args>
    void AddComponent(Args&& ...args)
    {
        m_EntityHandle.emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    void RemoveComponent()
    {
        m_EntityHandle.erase<T>();
    }

    const std::string& GetName() const;
    void SetName(const std::string& name);

    const Level* GetHomeLevel() const
    {
        return m_HomeLevel;
    }

    Level* GetHomeLevel()
    {
        return m_HomeLevel;
    }

    void DestroyActor();
    bool IsAlive() const;

    bool operator==(const Actor& other) const
    {
        return m_EntityHandle.entity() == other.m_EntityHandle.entity();
    }

    bool operator!=(const Actor& other) const
    {
        return m_EntityHandle.entity() != other.m_EntityHandle.entity();
    }

private:
    entt::handle m_EntityHandle;
    Level* m_HomeLevel{nullptr};
};

FORCE_INLINE void TransformComponent::Translate(const glm::vec3& pos)
{
    Position += pos;
}

FORCE_INLINE void TransformComponent::SetEulerAngles(const glm::vec3& eulerAngles)
{
    Rotation = glm::quat{glm::radians(eulerAngles)};
}

FORCE_INLINE void TransformComponent::SetEulerAngles(float pitch, float yaw, float roll)
{
    Rotation = glm::quat{glm::radians(glm::vec3{pitch, yaw, roll})};
}

FORCE_INLINE void TransformComponent::AddEulerAngles(const glm::vec3& eulerAngles)
{
    Rotation *= glm::quat{glm::vec3{eulerAngles}};
}

FORCE_INLINE glm::mat4 TransformComponent::GetWorldTransformMatrix() const
{
    return glm::translate(glm::identity<glm::mat4>(), Position) *
        glm::mat4_cast(Rotation) *
        glm::scale(glm::identity<glm::mat4>(), Scale);
}

FORCE_INLINE glm::vec3 TransformComponent::GetEulerAngles() const
{
    return glm::degrees(glm::eulerAngles(Rotation));
}

FORCE_INLINE glm::vec3 TransformComponent::GetForwardVector() const
{
    return glm::normalize(Rotation * glm::vec3{0, 0, -1});
}

FORCE_INLINE glm::vec3 TransformComponent::GetRightVector() const
{
    return glm::normalize(Rotation * glm::vec3{1, 0, 0});
}

FORCE_INLINE glm::vec3 TransformComponent::GetUpVector() const
{
    return glm::normalize(Rotation * glm::vec3{0, 1, 0});
}

FORCE_INLINE Transform TransformComponent::GetAsTransform() const
{
    return Transform{Position, Rotation, Scale};
}
