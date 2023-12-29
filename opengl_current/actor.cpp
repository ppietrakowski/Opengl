#include "actor.h"
#include "level.h"

glm::mat4 TransformComponent::GetWorldTransformMatrix() const
{
    const entt::handle* parentTraverseIt = &Parent;

    glm::mat4 worldTransform = CalculateRelativeTransform();

    while (parentTraverseIt->valid())
    {
        // accumulate all parent transforms
        const TransformComponent& t = parentTraverseIt->get<TransformComponent>();
        worldTransform = t.CalculateRelativeTransform() * worldTransform;
        parentTraverseIt = &t.Parent;
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
    hierarchy.AddChild(m_EntityHandle, actor.m_EntityHandle);
}

void Actor::RemoveChild(const Actor& actor)
{
    auto& hierarchy = GetComponent<SceneHierarchyComponent>();
    hierarchy.RemoveChild(actor.m_EntityHandle);
}

void Actor::DestroyActor()
{
    m_HomeLevel->RemoveActor(GetName());
}

Actor::Actor()
{
}
