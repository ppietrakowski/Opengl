#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"
#include "instanced_mesh_component.h"
#include "resouce_manager.h"
#include "player_controller.h"

#include <future>

Level::Level() :
    m_ResourceManager{ResourceManager::CreateResourceManager()}
{
}

Level::~Level()
{
    ResourceManager::Quit();
}

Actor Level::CreateActor(const std::string& name)
{
    Actor actor;

    actor.m_HomeLevel = this;
    actor.m_EntityHandle = entt::handle{m_Registry, m_Registry.create()};
    actor.AddComponent<TransformComponent>();
    actor.AddComponent<ActorTagComponent>();

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.Name = name;

    m_Actors[name] = actor;
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

    auto view = m_Registry.view<ActorTagComponent>();

    for (auto&& [entity, t] : view.each())
    {
        if (t.Tag == tag)
        {
            actorsWithTag.emplace_back(ConstructFromEntity(entity));
        }
    }

    return actorsWithTag;
}

void Level::RemoveActor(const std::string& name)
{
    auto it = m_Actors.find(name);

    if (it == m_Actors.end())
    {
        return;
    }

    Actor actor = it->second;

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.bIsAlive = false;

    actor.m_EntityHandle.destroy();
    m_Actors.erase(it);
}

void Level::StartupLevel()
{
}

void Level::BroadcastUpdate(Duration duration)
{
    // start all update tasks that are independent from themselfs
    auto skeletalAnimationUpdateTask = std::async(std::launch::async, [this](Duration duration) {
        UpdateSkeletalMeshesAnimation(duration);
    }, duration);


    auto playerControllerView = m_Registry.view<PlayerController>();

    for (auto&& [entity, playerController] : playerControllerView.each())
    {
        playerController.Update();
    }

    skeletalAnimationUpdateTask.wait();
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

    auto instancedMeshComponent = m_Registry.view<TransformComponent, InstancedMeshComponent>();
    for (auto&& [entity, transform, staticMesh] : instancedMeshComponent.each())
    {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }
}

void Level::UpdateSkeletalMeshesAnimation(Duration duration)
{
    auto skeletalMeshView = m_Registry.view<SkeletalMeshComponent>();

    float seconds = duration.GetSeconds();

    for (auto&& [entity, skeletalMesh] : skeletalMeshView.each())
    {
        skeletalMesh.UpdateAnimation(seconds);
    }
}
