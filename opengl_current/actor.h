#pragma once

#include "actor_tag_component.h"
#include "level_interface.h"
#include "transform_component.h"

#include <entt/entt.hpp>

class Level;

// trait of actor component
// specialize template with is_tickable=true to get ticks
template <typename T>
struct ActorTickTrait {
    static constexpr inline bool is_tickable = false;
};

#define DECLARE_COMPONENT_TICKABLE(ComponentClass) template<> struct ActorTickTrait<ComponentClass> { static constexpr inline bool is_tickable = true; }

typedef void(*ActorTickFn)(float delta_seconds, entt::handle& actor);

constexpr float kTickNonStop = 0.0f;

struct ActorTickFunction {
    ActorTickFn tick_fn;
    float tick_interval{kTickNonStop};
    float time_left_to_tick{0.0f};

    void ExecuteTick(float delta_seconds, entt::handle& actor) {
        if (tick_interval != kTickNonStop) {
            time_left_to_tick -= delta_seconds;

            if (time_left_to_tick == 0.0f) {
                tick_fn(delta_seconds, actor);
                time_left_to_tick = tick_interval;
            }
        } else {
            tick_fn(delta_seconds, actor);
        }
    }
};

struct ActorNativeTickable {
    std::vector<ActorTickFunction> tick_functions;
    entt::handle actor;

    template <typename T>
    void Bind(float interval = kTickNonStop) {
        ActorTickFunction function{};
        function.tick_fn = [](float delta_seconds, entt::handle& actor) {
            T& component = actor.get<T>();
            component.Tick(delta_seconds);
        };

        function.tick_interval = interval;
        
        tick_functions.emplace_back(function);
    }

    ActorNativeTickable(const entt::handle& actor) :
        actor(actor) {
    }

    ActorNativeTickable(const ActorNativeTickable&) = default;
    ActorNativeTickable& operator=(const ActorNativeTickable&) = default;

    void ExecuteTick(float delta_seconds);
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

        if constexpr(ActorTickTrait<T>::is_tickable) {
            auto& tickable = entity_handle_.get<ActorNativeTickable>();
            tickable.Bind<T>();
        }
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
