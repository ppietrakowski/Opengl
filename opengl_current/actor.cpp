#include "actor.h"
#include "level_interface.h"

#include "error_macros.h"

Actor::Actor(LevelInterface* level, const entt::handle& entity_handle) :
    home_level_{level},
    entity_handle_{entity_handle} {
}

const std::string& Actor::GetName() const {
    auto& tag_component = GetComponent<ActorTagComponent>();
    return tag_component.name;
}

void Actor::SetName(const std::string& name) {
    ASSERT(!name.empty());

    auto& tag_component = GetComponent<ActorTagComponent>();
    std::string old_name = tag_component.name;
    tag_component.name = name;
    
    home_level_->NotifyActorNameChanged(old_name, name);
}

void Actor::DestroyActor() {
    home_level_->RemoveActor(GetName());
}

bool Actor::IsAlive() const {
    return entity_handle_.valid();
}

void ActorNativeTickable::ExecuteTick(float delta_seconds) {

    for (ActorTickFunction& tick_function : tick_functions) {
        tick_function.ExecuteTick(delta_seconds, actor);
    }
}
