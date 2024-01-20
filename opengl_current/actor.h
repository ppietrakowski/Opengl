#pragma once

#include "actor_tag_component.h"
#include "level_interface.h"
#include "transform_component.h"

#include <entt/entt.hpp>

class Level;

template <typename T>
struct ActorTickTrait {
    static inline bool is_tickable = false;
};

// Basic gameplay object. This class is copy constructible
class Actor {
    friend class Level;
public:
    Actor() = default;
    Actor(LevelInterface* level, const entt::handle& entity_handle);
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

    const Level* GetHomeLevel() const {
        return (const Level*)home_level_;
    }

    Level* GetHomeLevel() {
        return (Level*)home_level_;
    }

    void DestroyActor();
    bool IsAlive() const;

    bool operator==(const Actor& other) const {
        return entity_handle_.entity() == other.entity_handle_.entity();
    }

    bool operator!=(const Actor& other) const {
        return entity_handle_.entity() != other.entity_handle_.entity();
    }

    const TransformComponent& GetTransform() const {
        return entity_handle_.get<TransformComponent>();
    }

    TransformComponent& GetTransform() {
        return entity_handle_.get<TransformComponent>();
    }

private:
    entt::handle entity_handle_;
    LevelInterface* home_level_{nullptr};
};
