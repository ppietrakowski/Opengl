#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"

#include <future>

Level::Level() :
    resource_manager_{ResourceManager::CreateResourceManager()} {
}

Level::~Level() {
    ResourceManager::Quit();
}

Actor Level::CreateActor(const std::string& name) {
    Actor actor;

    actor.home_level_ = this;
    actor.entity_handle_ = entt::handle{registry_, registry_.create()};
    actor.AddComponent<SceneHierarchyComponent>();
    actor.AddComponent<TransformComponent>();

    SceneHierarchyComponent& scene_hierarchy = actor.GetComponent<SceneHierarchyComponent>();
    scene_hierarchy.parent = entt::handle{registry_, entt::null};

    auto& transform = actor.GetComponent<TransformComponent>();
    transform.parent = entt::handle{registry_, entt::null};

    actor.AddComponent<ActorTagComponent>();

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.name = name;

    actors_.emplace_back(actor);

    std::sort(actors_.begin(), actors_.end(),
        [](const Actor& a, const Actor& b) {
        return std::less<std::string>()(a.GetName(), b.GetName());
    });

    return actor;
}

Actor Level::FindActor(const std::string& name) const {
    auto it = std::lower_bound(actors_.begin(), actors_.end(),
        name, [](const Actor& actor, const std::string& name) {
        return actor.GetName() < name;
    });

    CRASH_EXPECTED_TRUE(it != actors_.end());
    return *it;
}

std::vector<Actor> Level::FindActorsWithTag(const std::string& tag) const {
    std::vector<Actor> actors_with_tag;

    actors_with_tag.reserve(actors_.size());

    auto view = registry_.view<ActorTagComponent>();

    for (auto&& [entity, t] : view.each()) {
        if (t.tag == tag) {
            actors_with_tag.emplace_back(ConstructFromEntity(entity));
        }
    }

    return actors_with_tag;
}

void Level::RemoveActor(const std::string& name) {
    auto it = std::lower_bound(actors_.begin(), actors_.end(),
        name, [](const Actor& actor, const std::string& name) {
        return actor.GetName() < name;
    });

    if (it == actors_.end()) {
        return;
    }

    Actor actor = *it;

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.is_alive = false;

    registry_.destroy(actor.entity_handle_.entity());
    actors_.erase(it);
}

void Level::StartupLevel() {
}

void Level::BroadcastUpdate(Duration duration) {
    // start all update tasks that are independent from themselfs
    auto skeletal_update_task = std::async(std::launch::async, [this](Duration duration) {
        UpdateSkeletalMeshesAnimation(duration);
    }, duration);

    skeletal_update_task.wait();
}

void Level::BroadcastRender(Duration duration) {
    auto static_mesh_view = registry_.view<TransformComponent, StaticMeshComponent>();

    for (auto&& [entity, transform, static_mesh] : static_mesh_view.each()) {
        static_mesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto skeletal_mesh_view = registry_.view<TransformComponent, SkeletalMeshComponent>();
    for (auto&& [entity, transform, skeletal_mesh] : skeletal_mesh_view.each()) {
        skeletal_mesh.Draw(transform.GetWorldTransformMatrix());
    }
}

void Level::UpdateSkeletalMeshesAnimation(Duration duration) {
    auto skeletal_mesh_view = registry_.view<SkeletalMeshComponent>();

    for (auto&& [entity, skeletal_mesh] : skeletal_mesh_view.each()) {
        skeletal_mesh.UpdateAnimation(duration.GetAsSeconds());
    }
}
