#pragma once

#include "ActorTagComponent.hpp"
#include "LevelInterface.hpp"
#include "TransformComponent.hpp"
#include "Duration.hpp"

#include <entt/entt.hpp>

class Level;

// trait of actor component
// specialize template with bIsTickable=true to get ticks
template <typename T>
struct ActorTickTrait
{
    static constexpr inline bool bIsTickable = false;
};

template <typename T>
struct ActorSpawnedTrait
{
    static constexpr inline bool bSpawned = false;
};

#define DECLARE_COMPONENT_TICKABLE(ComponentClass) template<> struct ActorTickTrait<ComponentClass> { static constexpr inline bool bIsTickable = true; }

#define DECLARE_COMPONENT_SPAWNED(ComponentClass) template<> struct ActorSpawnedTrait<ComponentClass> { static constexpr inline bool bSpawned = true; }

typedef void(*ActorTickFn)(Duration deltaSeconds, entt::handle& actor);

constexpr float TickNonStop = 0.0f;

struct ActorTickFunction
{
    ActorTickFn TickFn;
    float TickInterval{TickNonStop};
    float TimeLeftToTick{0.0f};

    void ExecuteTick(Duration deltaSeconds, entt::handle& actor)
    {
        if (TickInterval != TickNonStop)
        {
            TimeLeftToTick -= deltaSeconds.GetSeconds();

            if (TimeLeftToTick == 0.0f)
            {
                TickFn(deltaSeconds, actor);
                TimeLeftToTick = TickInterval;
            }
        }
        else
        {
            TickFn(deltaSeconds, actor);
        }
    }
};

struct ActorNativeTickable
{
    std::vector<ActorTickFunction> TickFunctions;
    entt::handle TickableActor;

    template <typename T>
    void Bind(float interval = TickNonStop)
    {
        ActorTickFunction function{};
        function.TickFn = [](Duration deltaSeconds, entt::handle& actor)
        {
            T& component = actor.get<T>();
            component.Tick(deltaSeconds);
        };

        function.TickInterval = interval;

        TickFunctions.emplace_back(function);
    }

    ActorNativeTickable(const entt::handle& actor) :
        TickableActor(actor)
    {
    }

    ActorNativeTickable(const ActorNativeTickable&) = default;
    ActorNativeTickable& operator=(const ActorNativeTickable&) = default;

    void ExecuteTick(Duration deltaSeconds);
};

struct CameraComponent
{
    glm::vec3 Position;
    glm::quat Rotation;
};

class ActorComponent;
class Actor;

struct NativeComponentCollection
{
    using GetterFunc = ActorComponent* (*)(Actor& actor);
    std::vector<GetterFunc> NativeComponents;
};

// Basic gameplay object. This class is copy constructible
class Actor
{
    friend class Level;
public:
    Actor() = default;
    Actor(std::weak_ptr<LevelInterface> level, const entt::handle& entityHandle);
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

        if constexpr (ActorTickTrait<T>::bIsTickable)
        {
            auto& tickable = m_EntityHandle.get<ActorNativeTickable>();
            tickable.Bind<T>();
        }


        if constexpr (ActorSpawnedTrait<T>::bSpawned)
        {
            T& c = GetComponent<T>();
            c.Spawned();
        }

        if constexpr (std::is_base_of_v<ActorComponent, T>)
        {
            auto& c = GetComponent<NativeComponentCollection>();

            c.NativeComponents.emplace_back([](Actor& actor)
            {
                return static_cast<ActorComponent*>(&actor.GetComponent<T>());
            });

            c.NativeComponents.back()(*this)->m_Actor = *this;
        }
    }

    template <typename T>
    void RemoveComponent()
    {
        m_EntityHandle.erase<T>();
    }

    const std::string& GetName() const;
    void SetName(const std::string& name);

    std::shared_ptr<Level> GetHomeLevel() const;

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

    const TransformComponent& GetTransform() const
    {
        return m_EntityHandle.get<TransformComponent>();
    }

    TransformComponent& GetTransform()
    {
        return m_EntityHandle.get<TransformComponent>();
    }

private:
    entt::handle m_EntityHandle;
    std::weak_ptr<LevelInterface> m_HomeLevel;

    std::shared_ptr<LevelInterface> GetLevelImpl() const
    {
        return m_HomeLevel.lock();
    }
};
