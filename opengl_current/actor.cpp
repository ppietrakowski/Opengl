#include "actor.h"
#include "level.h"

glm::mat4 TransformComponent::GetWorldTransformMatrix() const
{
    entt::handle parentTraverseIt = Parent;

    glm::mat4 worldTransform = CalculateRelativeTransform();

    while (parentTraverseIt.valid())
    {
        // accumulate all parent transforms
        const TransformComponent& parentTransform = parentTraverseIt.get<TransformComponent>();
        worldTransform = parentTransform.CalculateRelativeTransform() * worldTransform;
        parentTraverseIt = parentTransform.Parent;
    }

    return worldTransform;
}

void SceneHierarchyComponent::RemoveChild(entt::handle handle)
{
    const ActorTagComponent& tagComponent = handle.get<ActorTagComponent>();
    auto it = Children.find(tagComponent.Name);
    assert(it != Children.end());

    it->second.get<TransformComponent>().Parent = entt::handle{};
    Children.erase(it);
}

void SceneHierarchyComponent::AddChild(const entt::handle& self, entt::handle handle)
{
    const ActorTagComponent& tagComponent = handle.get<ActorTagComponent>();
    Children[tagComponent.Name] = handle;
    handle.get<TransformComponent>().Parent = self;
}

void SceneHierarchyComponent::InvalidateState()
{
    auto predicate = [](const std::pair<const std::string, entt::handle>& a) { return !a.second.valid(); };

    auto it = std::find_if(Children.begin(), Children.end(), predicate);

    while (it != Children.end())
    {
        Children.erase(it);
        it = std::find_if(Children.begin(), Children.end(), predicate);
    }
}

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

void Actor::AddChild(const Actor& actor)
{
    auto& hierarchy = GetComponent<SceneHierarchyComponent>();
    hierarchy.AddChild(EntityHandle, actor.EntityHandle);
}

void Actor::RemoveChild(const Actor& actor)
{
    auto& hierarchy = GetComponent<SceneHierarchyComponent>();
    hierarchy.RemoveChild(actor.EntityHandle);
}

void Actor::DestroyActor()
{
    auto& sceneHierarchy = GetComponent<SceneHierarchyComponent>();
    for (auto& [name, actor] : sceneHierarchy.Children)
    {
        HomeLevel->RemoveActor(name);
    }

    HomeLevel->RemoveActor(GetName());
}

bool Actor::IsAlive() const
{
    return EntityHandle.valid();
}

Actor::Actor()
{
}
