#include "actor.h"
#include "level_interface.h"

#include "error_macros.h"

Actor::Actor(LevelInterface* level, const entt::handle& entityHandle) :
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

    m_HomeLevel->NotifyActorNameChanged(oldName, name);
}

void Actor::DestroyActor()
{
    m_HomeLevel->RemoveActor(GetName());
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
