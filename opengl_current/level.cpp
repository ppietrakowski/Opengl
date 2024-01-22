#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"
#include "instanced_mesh_component.h"
#include "resouce_manager.h"
#include "player_controller.h"
#include "light_component.h"

#include <future>

Level::Level() :
    resource_manager_{ResourceManager::CreateResourceManager()} {
}

Level::~Level() {
    ResourceManager::Quit();
}

Actor Level::CreateActor(const std::string& name) {
    Actor actor(this, entt::handle{registry_, registry_.create()});
    actor.AddComponent<TransformComponent>();
    actor.AddComponent<ActorTagComponent>();
    actor.AddComponent<ActorNativeTickable>(actor.entity_handle_);

    auto& tag = actor.GetComponent<ActorTagComponent>();
    tag.name = name;

    actors_[name] = actor;
    return actor;
}

Actor Level::FindActor(const std::string& name) const {
    return actors_.at(name);
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
    auto it = actors_.find(name);

    if (it == actors_.end()) {
        return;
    }

    Actor actor = it->second;

    auto& tag = actor.GetComponent<ActorTagComponent>();
    actor.entity_handle_.destroy();
    actors_.erase(it);
}

void Level::NotifyActorNameChanged(const std::string& old_name, const std::string& new_name) {
    Actor actor = actors_[old_name];

    if (actors_.contains(new_name)) {
        actors_.erase(old_name);
        actors_[new_name] = actor;
    }
}

void Level::StartupLevel() {
}

void Level::BroadcastUpdate(Duration duration) {
    // start all update tasks that are independent from themselfs
    auto skeletal_animation_update_task = std::async(std::launch::async, [this](Duration duration) {
        UpdateSkeletalMeshesAnimation(duration);
    }, duration);

    auto player_controller_view = registry_.view<PlayerController>();

    for (auto&& [entity, playerController] : player_controller_view.each()) {
        playerController.Update();
    }

    auto camera = registry_.view<CameraComponent, TransformComponent>();

    for (auto&& [entity, camera_component, transform] : camera.each()) {
        camera_component.pos = transform.position;
        camera_component.rotation = transform.rotation;
    }

    auto actor_tick_functions = registry_.view<ActorNativeTickable>();

    float delta_seconds = duration.GetSeconds();

    for (auto&& [entity, tick_function] : actor_tick_functions.each()) {
        tick_function.ExecuteTick(delta_seconds);
    }

    skeletal_animation_update_task.wait();
}

void Level::BroadcastRender(Duration duration) {
    auto static_mesh_view = registry_.view<TransformComponent, StaticMeshComponent>();

    auto directional_light_view = registry_.view<DirectionalLightComponent, TransformComponent>();

    for (auto&& [entity, directional_light, transform] : directional_light_view.each()) {
        glm::vec3 transformed_direction = transform.rotation * directional_light.direction;

        LightData light_data{transform.position, 1.0f, transformed_direction,
            1.0f, directional_light.color, 0.0f, LightType::Directional, 0.0f, 1.0f};

        Renderer::AddLight(light_data);
    }

    auto point_light_view = registry_.view<PointLightComponent, TransformComponent>();

    for (auto&& [entity, point_light, transform] : point_light_view.each()) {
        glm::vec3 transformed_direction = transform.rotation * point_light.direction;

        LightData light_data{transform.position, 1.0f, transformed_direction,
            1.0f, point_light.color, point_light.direction_length, LightType::Point, 0.0f, point_light.intensity};
        Renderer::AddLight(light_data);
    }

    auto spot_light_view = registry_.view<SpotLightComponent, TransformComponent>();

    for (auto&& [entity, spot_light, transform] : spot_light_view.each()) {
        glm::vec3 transformed_direction = transform.rotation * spot_light.direction;

        LightData light_data{transform.position, 1.0f, transformed_direction,
            1.0f, spot_light.color, spot_light.direction_length, LightType::Spot, cosf(glm::radians(spot_light.cut_off_angle)), spot_light.intensity};
        Renderer::AddLight(light_data);
    }

    for (auto&& [entity, transform, staticMesh] : static_mesh_view.each()) {
        AddNewStaticMesh(staticMesh.mesh_name, transform.GetAsTransform());
    }

    for (auto& [name, mesh] : instanced_mesh_) {
        mesh->Draw(glm::mat4{1.0f});
        mesh->Clear();
    }

    auto skeletal_mesh_view = registry_.view<TransformComponent, SkeletalMeshComponent>();
    for (auto&& [entity, transform, skeletal_mesh] : skeletal_mesh_view.each()) {
        skeletal_mesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto instanced_mesh_component = registry_.view<TransformComponent, InstancedMeshComponent>();
    for (auto&& [entity, transform, staticMesh] : instanced_mesh_component.each()) {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }
}

void Level::AddNewStaticMesh(const std::string& mesh_name, const Transform& transform) {
    auto it = instanced_mesh_.find(mesh_name);

    if (it == instanced_mesh_.end()) {
        it = instanced_mesh_.try_emplace(mesh_name, std::make_shared<InstancedMesh>(ResourceManager::GetStaticMesh(mesh_name),
            ResourceManager::GetMaterial("instanced"))).first;
    }

    auto& mesh = it->second->GetMesh();

    if (Renderer::IsVisibleToCamera(transform.position, mesh.GetBBoxMin(), mesh.GetBBoxMax())) {
        it->second->AddInstance(transform, 0);
    }
}


bool Level::TryFindActor(const std::string& name, Actor& out_actor) {
    auto it = actors_.find(name);

    if (it != actors_.end()) {
        out_actor = it->second;
        return true;
    }

    return false;
}

const CameraComponent& Level::FindCameraComponent() const {
    auto view = View<CameraComponent>();

    return registry_.get<CameraComponent>(view.front());
}

void Level::UpdateSkeletalMeshesAnimation(Duration duration) {
    auto skeletal_mesh_view = registry_.view<SkeletalMeshComponent, TransformComponent>();

    float seconds = duration.GetSeconds();

    for (auto&& [entity, skeletal_mesh, transform] : skeletal_mesh_view.each()) {
        skeletal_mesh.UpdateAnimation(seconds, transform.GetAsTransform());
    }
}
