#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"

#include <future>

Level::Level():
    ResourceManagerInstance{ResourceManager::CreateResourceManager()}
{
}

Level::~Level()
{
    ResourceManager::Quit();
}

Actor Level::CreateActor(const std::string& name)
{
    Actor actor;

    actor.HomeLevel = this;
    actor.EntityHandle = entt::handle{Registry, Registry.create()};
    actor.AddComponent<SceneHierarchyComponent>();
    actor.AddComponent<TransformComponent>();

    SceneHierarchyComponent& sceneHierarchy = actor.GetComponent<SceneHierarchyComponent>();
    sceneHierarchy.Parent = entt::handle{Registry, entt::null};

    auto& transform = actor.GetComponent<TransformComponent>();
    transform.Parent = entt::handle{Registry, entt::null};

    actor.AddComponent<ActorTagComponent>();

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.Name = name;

    Actors.try_emplace(name, actor);
    return actor;
}

Actor Level::FindActor(const std::string& name) const
{
    return Actors.at(name);
}

std::vector<Actor> Level::FindActorsWithTag(const std::string& tag) const
{
    std::vector<Actor> actorsWithTag;

    actorsWithTag.reserve(Actors.size());

    for (auto& [name, actor] : Actors)
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
    auto& actor = Actors.at(name);
    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.IsAlive = false;

    Registry.destroy(actor.EntityHandle.entity());
    Actors.erase(name);
}

void Level::StartupLevel()
{
}

void Level::BroadcastUpdate(Duration duration)
{
    // start all update tasks that are independent from themselfs
    auto skeletalUpdateTask = std::async(std::launch::async, [this](Duration duration) {
        UpdateSkeletalMeshesAnimation(duration);
    }, duration);

    skeletalUpdateTask.wait();
}

void Level::BroadcastRender(Duration duration)
{
    auto staticMeshView = Registry.view<TransformComponent, StaticMeshComponent>();

    for (auto&& [entity, transform, staticMesh] : staticMeshView.each())
    {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto skeletalMeshView = Registry.view<TransformComponent, SkeletalMeshComponent>();
    for (auto&& [entity, transform, skeletalMesh] : skeletalMeshView.each())
    {
        skeletalMesh.Draw(transform.GetWorldTransformMatrix());
    }
}

void Level::UpdateSkeletalMeshesAnimation(Duration duration)
{
    auto skeletalMeshView = Registry.view<SkeletalMeshComponent>();

    for (auto&& [entity, skeletalMesh] : skeletalMeshView.each())
    {
        skeletalMesh.UpdateAnimation(duration.GetAsSeconds());
    }
}
