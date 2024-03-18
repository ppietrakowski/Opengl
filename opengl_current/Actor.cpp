#include "Actor.hpp"
#include "LevelInterface.hpp"

#include "ErrorMacros.hpp"

Actor::Actor(std::weak_ptr<LevelInterface> level, const entt::handle& entityHandle) :
    m_HomeLevel{level},
    m_EntityHandle{entityHandle}
{
}

const std::string& Actor::GetName() const
{
    auto& tag_component = GetComponent<ActorTagComponent>();
    return tag_component.Name;
}

void Actor::SetName(const std::string& name)
{
    ASSERT(!name.empty());

    auto& tag_component = GetComponent<ActorTagComponent>();
    std::string oldName = tag_component.Name;
    tag_component.Name = name;

    GetLevelImpl()->NotifyActorNameChanged(oldName, name);
}

std::shared_ptr<Level> Actor::GetHomeLevel() const
{
    return std::reinterpret_pointer_cast<Level>(GetLevelImpl());
}

void Actor::DestroyActor()
{
    GetLevelImpl()->RemoveActor(GetName());
}

bool Actor::IsAlive() const
{
    return m_EntityHandle.valid();
}

void ActorNativeTickable::ExecuteTick(Duration deltaSeconds)
{

    for (ActorTickFunction& tick_function : TickFunctions)
    {
        tick_function.ExecuteTick(deltaSeconds, TickableActor);
    }
}
