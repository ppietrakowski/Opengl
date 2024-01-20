#include "actor.h"
#include "level_interface.h"

Actor::Actor(LevelInterface* level, const entt::handle& entity_handle) :
    home_level_{level},
    entity_handle_{entity_handle} {
}

const std::string& Actor::GetName() const {
    auto& tagComponent = GetComponent<ActorTagComponent>();
    return tagComponent.name;
}

void Actor::SetName(const std::string& name) {
    auto& tagComponent = GetComponent<ActorTagComponent>();
    tagComponent.name = name;
}

void Actor::DestroyActor() {
    home_level_->RemoveActor(GetName());
}

bool Actor::IsAlive() const {
    return entity_handle_.valid();
}
