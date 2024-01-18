#include "actor.h"
#include "level.h"
#include "error_macros.h"

#include <glm/gtx/matrix_decompose.hpp>

const std::string& Actor::GetName() const
{
    auto& tagComponent = GetComponent<ActorTagComponent>();
    return tagComponent.name;
}

void Actor::SetName(const std::string& name)
{
    auto& tagComponent = GetComponent<ActorTagComponent>();
    tagComponent.name = name;
}

void Actor::DestroyActor()
{
    home_level_->RemoveActor(GetName());
}

bool Actor::IsAlive() const
{
    return entity_handle_.valid();
}

Actor::Actor()
{
}
