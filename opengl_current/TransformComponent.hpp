#pragma once

#include "Transform.hpp"
#include "Datapack.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class TransformComponent
{
public:
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


    Datapack Archived() const;
    void Unarchive(const Datapack& datapack);
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

inline Datapack TransformComponent::Archived() const
{
    Datapack pack;
    Serialize(Scale, pack["Scale"]);
    Serialize(Position, pack["Position"]);
    Serialize(Rotation, pack["Rotation"]);
    return pack;
}

inline void TransformComponent::Unarchive(const Datapack& datapack)
{
    for (int i = 0; i < Scale.length(); ++i)
    {
        Scale[i] = datapack.GetSafe("Scale").AsFloatNumber(i);
        Position[i] = datapack.GetSafe("Position").AsFloatNumber(i);
    }

    for (int i = 0; i < Rotation.length(); ++i)
    {
        Rotation[i] = datapack.GetSafe("Rotation").AsFloatNumber(i);
    }
}
