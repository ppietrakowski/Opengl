#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct ActorTagComponent {
    std::string name;
    std::string tag{"Default"};
    bool is_alive{true};
};

struct TransformComponent {
    entt::handle parent;

    // Position in parent's space (World space, if parent unspecified)
    glm::vec3 position{0, 0, 0};

    // Rotation in parent's space (World space, if parent unspecified)
    glm::quat rotation{glm::vec3{0, 0, 0}};

    glm::vec3 scale{1, 1, 1};

    glm::mat4 GetWorldTransformMatrix() const;

    glm::mat4 CalculateRelativeTransform() const {
        return glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
    }

    void SetLocalEulerAngles(const glm::vec3& euler_angles) {
        rotation = glm::quat{glm::radians(euler_angles)};
    }

    glm::vec3 GetWorldPosition() const;
};

struct SceneHierarchyComponent {
    entt::handle parent;
    std::map<std::string, entt::handle> children;

    void RemoveChild(entt::handle handle);
    void AddChild(const entt::handle& self, entt::handle handle);

    void InvalidateState();
};

class Level;

// Basic gameplay object. This class is copy constructible
class Actor {
    friend class Level;
public:
    Actor();
    Actor(const Actor&) = default;
    Actor& operator=(const Actor&) = default;

    template <typename T>
    T& GetComponent() {
        return entity_handle_.get<T>();
    }

    template <typename T>
    const T& GetComponent() const {
        return entity_handle_.get<T>();
    }

    template <typename T, typename ...Args>
    void AddComponent(Args&& ...args) {
        entity_handle_.emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    void RemoveComponent() {
        entity_handle_.erase<T>();
    }

    const std::string& GetName() const;
    void SetName(const std::string& name);

    void AddChild(const Actor& actor);
    void RemoveChild(const Actor& actor);

    const Level* GetHomeLevel() const {
        return home_level_;
    }

    Level* GetHomeLevel() {
        return home_level_;
    }

    void DestroyActor();
    bool IsAlive() const;

private:
    entt::handle entity_handle_;
    Level* home_level_{nullptr};
};

