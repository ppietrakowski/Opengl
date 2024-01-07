#include "actor.h"
#include "level.h"

glm::mat4 TransformComponent::GetWorldTransformMatrix() const {
    entt::handle parent_traverse_it = parent;

    glm::mat4 world_transform = CalculateRelativeTransform();

    while (parent_traverse_it.valid()) {
        // accumulate all parent transforms
        const TransformComponent& parent_transform = parent_traverse_it.get<TransformComponent>();
        world_transform = parent_transform.CalculateRelativeTransform() * world_transform;
        parent_traverse_it = parent_transform.parent;
    }

    return world_transform;
}

void SceneHierarchyComponent::RemoveChild(entt::handle handle) {
    const ActorTagComponent& tag_component = handle.get<ActorTagComponent>();
    auto it = children.find(tag_component.name);
    assert(it != children.end());

    it->second.get<TransformComponent>().parent = entt::handle{};
    children.erase(it);
}

void SceneHierarchyComponent::AddChild(const entt::handle& self, entt::handle handle) {
    const ActorTagComponent& tag_component = handle.get<ActorTagComponent>();
    children[tag_component.name] = handle;
    handle.get<TransformComponent>().parent = self;
}

void SceneHierarchyComponent::InvalidateState() {
    auto predicate = [](const std::pair<const std::string, entt::handle>& a) { return !a.second.valid(); };

    auto it = std::find_if(children.begin(), children.end(), predicate);

    while (it != children.end()) {
        children.erase(it);
        it = std::find_if(children.begin(), children.end(), predicate);
    }
}

const std::string& Actor::GetName() const {
    auto& tag_component = GetComponent<ActorTagComponent>();
    return tag_component.name;
}

void Actor::SetName(const std::string& name) {
    auto& tag_component = GetComponent<ActorTagComponent>();
    tag_component.name = name;
}

void Actor::AddChild(const Actor& actor) {
    auto& hierarchy = GetComponent<SceneHierarchyComponent>();
    hierarchy.AddChild(entity_handle_, actor.entity_handle_);
}

void Actor::RemoveChild(const Actor& actor) {
    auto& hierarchy = GetComponent<SceneHierarchyComponent>();
    hierarchy.RemoveChild(actor.entity_handle_);
}

void Actor::DestroyActor() {
    auto& scene_hierarchy = GetComponent<SceneHierarchyComponent>();
    for (auto& [name, actor] : scene_hierarchy.children) {
        home_level_->RemoveActor(name);
    }

    home_level_->RemoveActor(GetName());
}

bool Actor::IsAlive() const {
    return entity_handle_.valid();
}

Actor::Actor() {
}
