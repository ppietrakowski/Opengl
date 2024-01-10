#pragma once

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
    entt::handle Parent;

    // Position in parent's space (World space, if parent unspecified)
    glm::vec3 Position{0, 0, 0};

    // Rotation in parent's space (World space, if parent unspecified)
    glm::quat Rotation{glm::vec3{0, 0, 0}};

    glm::vec3 Scale{1, 1, 1};

    glm::mat4 GetWorldTransformMatrix() const;

    glm::mat4 CalculateRelativeTransform() const
    {
        return glm::translate(glm::identity<glm::mat4>(), Position) * glm::mat4_cast(Rotation) * glm::scale(glm::identity<glm::mat4>(), Scale);
    }

    void SetLocalEulerAngles(const glm::vec3& eulerAngles)
    {
        Rotation = glm::quat{glm::radians(eulerAngles)};
    }

    glm::vec3 GetWorldPosition() const;
};

struct SceneHierarchyComponent
{
    entt::handle Parent;
    std::map<std::string, entt::handle> Children;

    void RemoveChild(entt::handle handle);
    void AddChild(const entt::handle& self, entt::handle handle);

    void InvalidateState();
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

    void AddChild(const Actor& actor);
    void RemoveChild(const Actor& actor);

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

private:
    entt::handle m_EntityHandle;
    Level* m_HomeLevel{nullptr};
};

