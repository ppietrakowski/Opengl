#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"

Actor Level::CreateActor(const std::string& name)
{
    Actor actor;

    actor.m_HomeLevel = this;
    actor.m_EntityHandle = entt::handle{m_Registry, m_Registry.create()};
    actor.AddComponent<SceneHierarchyComponent>();
    actor.AddComponent<TransformComponent>();

    SceneHierarchyComponent& sceneHierarchy = actor.GetComponent<SceneHierarchyComponent>();
    sceneHierarchy.Parent = entt::handle{m_Registry, entt::null};

    auto& transform = actor.GetComponent<TransformComponent>();
    transform.Parent = entt::handle{m_Registry, entt::null};

    actor.AddComponent<ActorTagComponent>();

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.Name = name;

    m_Actors.try_emplace(name, actor);
    return actor;
}

Actor Level::FindActor(const std::string& name) const
{
    return m_Actors.at(name);
}

std::vector<Actor> Level::FindActorsWithTag(const std::string& tag) const
{
    std::vector<Actor> actorsWithTag;
    
    actorsWithTag.reserve(m_Actors.size());

    for (auto& [name, actor] : m_Actors)
    {
        const ActorTagComponent& t = actor.GetComponent<ActorTagComponent>();
    
        if (t.Tag == tag)
        {
            actorsWithTag.emplace_back(actor);
        }
    }

    return actorsWithTag;
}

void Level::RemoveActor(const std::string& name)
{
    auto& actor = m_Actors.at(name);
    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.IsAlive = false;
}

void Level::StartupLevel()
{
}

void Level::BroadcastUpdate(Duration duration)
{
    auto skeletalMeshView = m_Registry.view<SkeletalMeshComponent>();

    for (auto&& [entity, skeletalMesh] : skeletalMeshView.each())
    {
        skeletalMesh.UpdateAnimation(duration.GetAsSeconds());
    }
}

void Level::BroadcastRender(Duration duration)
{
    auto staticMeshView = m_Registry.view<TransformComponent, StaticMeshComponent>();

    for (auto&& [entity, transform, staticMesh] : staticMeshView.each())
    {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto skeletalMeshView = m_Registry.view<TransformComponent, SkeletalMeshComponent>();
    for (auto&& [entity, transform, skeletalMesh] : skeletalMeshView.each())
    {
        skeletalMesh.Draw(transform.GetWorldTransformMatrix());
    }
}
