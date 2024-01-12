#include "actor.h"
#include "level.h"
#include "error_macros.h"

#include <glm/gtx/matrix_decompose.hpp>

const std::string& Actor::GetName() const
{
    auto& tagComponent = GetComponent<ActorTagComponent>();
    return tagComponent.Name;
}

void Actor::SetName(const std::string& name)
{
    auto& tagComponent = GetComponent<ActorTagComponent>();
    tagComponent.Name = name;
}

void Actor::DestroyActor()
{
    m_HomeLevel->RemoveActor(GetName());
}

bool Actor::IsAlive() const
{
    return m_EntityHandle.valid();
}

Actor::Actor()
{
}
