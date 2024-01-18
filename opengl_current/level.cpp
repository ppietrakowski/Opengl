#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"
#include "instanced_mesh_component.h"
#include "resouce_manager.h"
#include "player_controller.h"

#include <future>

Level::Level() :
    resource_manager_{ResourceManager::CreateResourceManager()}
{
}

Level::~Level()
{
    ResourceManager::Quit();
}

Actor Level::CreateActor(const std::string& name)
{
    Actor actor;

    actor.home_level_ = this;
    actor.entity_handle_ = entt::handle{registry_, registry_.create()};
    actor.AddComponent<TransformComponent>();
    actor.AddComponent<ActorTagComponent>();

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.name = name;

    actors_[name] = actor;
    return actor;
}

Actor Level::FindActor(const std::string& name) const
{
    return actors_.at(name);
}

std::vector<Actor> Level::FindActorsWithTag(const std::string& tag) const
{
    std::vector<Actor> actors_with_tag;

    actors_with_tag.reserve(actors_.size());

    auto view = registry_.view<ActorTagComponent>();

    for (auto&& [entity, t] : view.each())
    {
        if (t.tag == tag)
        {
            actors_with_tag.emplace_back(ConstructFromEntity(entity));
        }
    }

    return actors_with_tag;
}

void Level::RemoveActor(const std::string& name)
{
    auto it = actors_.find(name);

    if (it == actors_.end())
    {
        return;
    }

    Actor actor = it->second;

    auto& tag = actor.GetComponent<ActorTagComponent>();
    actor.entity_handle_.destroy();
    actors_.erase(it);
}

void Level::StartupLevel()
{
}

void Level::BroadcastUpdate(Duration duration)
{
    // start all update tasks that are independent from themselfs
    auto skeletal_animation_update_task = std::async(std::launch::async, [this](Duration duration) {
        UpdateSkeletalMeshesAnimation(duration);
    }, duration);

    auto player_controller_view = registry_.view<PlayerController>();

    for (auto&& [entity, playerController] : player_controller_view.each())
    {
        playerController.Update();
    }

    skeletal_animation_update_task.wait();
}

void Level::BroadcastRender(Duration duration)
{
    auto static_mesh_view = registry_.view<TransformComponent, StaticMeshComponent>();

    for (auto&& [entity, transform, staticMesh] : static_mesh_view.each())
    {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto skeletal_mesh_view = registry_.view<TransformComponent, SkeletalMeshComponent>();
    for (auto&& [entity, transform, skeletal_mesh] : skeletal_mesh_view.each())
    {
        skeletal_mesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto instanced_mesh_component = registry_.view<TransformComponent, InstancedMeshComponent>();
    for (auto&& [entity, transform, staticMesh] : instanced_mesh_component.each())
    {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }
}

void Level::UpdateSkeletalMeshesAnimation(Duration duration)
{
    auto skeletal_mesh_view = registry_.view<SkeletalMeshComponent, TransformComponent>();

    float seconds = duration.GetSeconds();

    for (auto&& [entity, skeletal_mesh, transform] : skeletal_mesh_view.each())
    {
        skeletal_mesh.UpdateAnimation(seconds, transform.GetAsTransform());
    }
}
