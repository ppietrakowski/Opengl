#pragma once

#include "actor.h"
#include "duration.h"

#include "instanced_mesh.h"
#include "lights.h"

class ResourceManagerImpl;

class Level : public LevelInterface
{
public:
    Level();
    ~Level();

    Actor CreateActor(const std::string& name);
    Actor FindActor(const std::string& name) const;
    std::vector<Actor> FindActorsWithTag(const std::string& tag) const;

    void RemoveActor(const std::string& name) override;
    void NotifyActorNameChanged(const std::string& oldName, const std::string& newName) override;

    void StartupLevel();
    void BroadcastUpdate(Duration duration);
    void BroadcastRender();

    auto begin()
    {
        return m_Actors.begin();
    }

    auto end()
    {
        return m_Actors.end();
    }

    template <typename ...Args>
    auto View()
    {
        return m_Registry.view<Args...>();
    }

    template <typename ...Args>
    const auto View() const
    {
        return m_Registry.view<Args...>();
    }

    void AddNewStaticMesh(const std::string& meshName, const Transform& transform);

    bool TryFindActor(const std::string& name, Actor& outActor);

    const CameraComponent& FindCameraComponent() const;

    glm::vec3 CameraPosition{0, 0, 0};
    glm::quat CameraRotation{glm::vec3{0, 0,0}};

    const std::vector<LightData>& GetLightsData() const
    {
        return m_Lights;
    }

private:
    entt::registry m_Registry;
    std::map<std::string, Actor> m_Actors;
    std::shared_ptr<ResourceManagerImpl> m_ResourceManager;

    std::unordered_map<std::string, std::shared_ptr<InstancedMesh>> m_MeshNameToInstancedMesh;

    std::vector<LightData> m_Lights;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);

    Actor ConstructFromEntity(entt::entity entity) const
    {
        return Actor{const_cast<Level*>(this), entt::handle{const_cast<entt::registry&>(m_Registry), entity}};
    }
};

