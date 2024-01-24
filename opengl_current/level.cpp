#include "level.h"
#include "static_mesh_component.h"
#include "skeletal_mesh_component.h"
#include "instanced_mesh_component.h"
#include "resouce_manager.h"
#include "player_controller.h"
#include "light_component.h"

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
    Actor actor(this, entt::handle{m_Registry, m_Registry.create()});
    actor.AddComponent<TransformComponent>();
    actor.AddComponent<ActorTagComponent>();
    actor.AddComponent<ActorNativeTickable>(actor.m_EntityHandle);

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
    actor.m_EntityHandle.destroy();
    m_Actors.erase(it);
}

void Level::NotifyActorNameChanged(const std::string& oldName, const std::string& newName)
{
    Actor actor = m_Actors[oldName];

    if (m_Actors.contains(newName))
    {
        m_Actors.erase(oldName);
        m_Actors[newName] = actor;
    }
}

void Level::StartupLevel()
{
}

void Level::BroadcastUpdate(Duration duration)
{
    // start all update tasks that are independent from themselfs
    auto skeletalAnimationUpdateTask = std::async(std::launch::async, [this](Duration duration)
    {
        UpdateSkeletalMeshesAnimation(duration);
    }, duration);

    auto playerControllerView = m_Registry.view<PlayerController>();

    for (auto&& [entity, playerController] : playerControllerView.each())
    {
        playerController.Update();
    }

    auto cameraView = m_Registry.view<CameraComponent, TransformComponent>();

    for (auto&& [entity, cameraComponent, transform] : cameraView.each())
    {
        cameraComponent.Position = transform.Position;
        cameraComponent.Rotation = transform.Rotation;

        CameraPosition = transform.Position;
        CameraRotation = transform.Rotation;
    }

    auto actorTickFunctionView = m_Registry.view<ActorNativeTickable>();

    for (auto&& [entity, tick_function] : actorTickFunctionView.each())
    {
        tick_function.ExecuteTick(duration);
    }

    skeletalAnimationUpdateTask.wait();
}

void Level::BroadcastRender()
{
    m_Lights.clear();

    auto directionalLightView = m_Registry.view<DirectionalLightComponent, TransformComponent>();

    for (auto&& [entity, directionalLight, transform] : directionalLightView.each())
    {
        glm::vec3 transformedDirection = transform.Rotation * directionalLight.Direction;

        LightData lightData{transform.Position, 1.0f, transformedDirection,
            1.0f, directionalLight.Color, 0.0f, LightType::Directional, 0.0f, 1.0f};

        m_Lights.emplace_back(lightData);
    }

    auto pointLightView = m_Registry.view<PointLightComponent, TransformComponent>();

    for (auto&& [entity, pointLight, transform] : pointLightView.each())
    {

        LightData lightData{transform.Position, 1.0f, glm::vec3{0.0f},
            1.0f, pointLight.Color, pointLight.DirectionLength, LightType::Point, 0.0f, pointLight.Intensity};

        m_Lights.emplace_back(lightData);
    }

    auto spot_light_view = m_Registry.view<SpotLightComponent, TransformComponent>();

    for (auto&& [entity, spot_light, transform] : spot_light_view.each())
    {
        glm::vec3 transformedDirection = transform.Rotation * spot_light.Direction;

        LightData lightData{transform.Position, 1.0f, transformedDirection,
            1.0f, spot_light.Color, spot_light.DirectionLength, LightType::Spot, cosf(glm::radians(spot_light.CutOffAngle)), spot_light.Intensity,
        cosf(glm::radians(spot_light.OuterCutOffAngle))
        };

        m_Lights.emplace_back(lightData);
    }

    auto staticMeshView = View<TransformComponent, StaticMeshComponent>();
    for (auto&& [entity, transform, staticMesh] : staticMeshView.each())
    {
        AddNewStaticMesh(staticMesh.MeshName, transform.GetAsTransform());
    }

    for (auto& [name, mesh] : m_MeshNameToInstancedMesh)
    {
        mesh->Draw(glm::mat4{1.0f});
        mesh->Clear();
    }

    auto skeletalMeshView = m_Registry.view<TransformComponent, SkeletalMeshComponent>();
    for (auto&& [entity, transform, skeletalMesh] : skeletalMeshView.each())
    {
        skeletalMesh.Draw(transform.GetWorldTransformMatrix());
    }

    auto instancedMeshComponentView = m_Registry.view<TransformComponent, InstancedMeshComponent>();
    for (auto&& [entity, transform, staticMesh] : instancedMeshComponentView.each())
    {
        staticMesh.Draw(transform.GetWorldTransformMatrix());
    }
}

void Level::AddNewStaticMesh(const std::string& meshName, const Transform& transform)
{
    auto it = m_MeshNameToInstancedMesh.find(meshName);

    if (it == m_MeshNameToInstancedMesh.end())
    {
        it = m_MeshNameToInstancedMesh.try_emplace(meshName, std::make_shared<InstancedMesh>(ResourceManager::GetStaticMesh(meshName),
            ResourceManager::GetMaterial("instanced"))).first;
    }

    auto& mesh = it->second->GetMesh();
    it->second->AddInstance(transform, 0);
}


bool Level::TryFindActor(const std::string& name, Actor& outActor)
{
    auto it = m_Actors.find(name);

    if (it != m_Actors.end())
    {
        outActor = it->second;
        return true;
    }

    return false;
}

const CameraComponent& Level::FindCameraComponent() const
{
    auto view = View<CameraComponent>();

    return m_Registry.get<CameraComponent>(view.front());
}

void Level::UpdateSkeletalMeshesAnimation(Duration duration)
{
    auto skeletalMeshView = m_Registry.view<SkeletalMeshComponent, TransformComponent>();

    float seconds = duration.GetSeconds();

    for (auto&& [entity, skeletalMesh, transform] : skeletalMeshView.each())
    {
        skeletalMesh.UpdateAnimation(seconds, transform.GetAsTransform());
    }
}
