#pragma once

#include "Actor.hpp"
#include "Duration.hpp"
#include "Entity.hpp"
#include "StaticMeshEntity.hpp"

#include "InstancedMesh.hpp"
#include "Lights.hpp"

#include "Archive.hpp"

#include <optional>

class ResourceManagerImpl;

class Level : public LevelInterface, public std::enable_shared_from_this<Level>
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

    std::optional<Actor> TryFindActor(const std::string& name);

    const CameraComponent& FindCameraComponent() const;

    glm::vec3 CameraPosition{0, 0, 0};
    glm::quat CameraRotation{glm::vec3{0, 0,0}};

    const std::vector<LightData>& GetLightsData() const
    {
        return m_Lights;
    }

    void SaveLevel(std::string_view path);

    template <typename T>
    std::shared_ptr<T> CreateEntity(std::string_view name)
    {
        std::shared_ptr<BaseEntity> obj = std::dynamic_pointer_cast<BaseEntity>(T::CreateNewInstance(name));

        if constexpr (std::is_same_v<T, StaticMeshEntity>)
        {
            m_StaticMeshEntity.emplace_back(std::dynamic_pointer_cast<StaticMeshEntity>(obj));
        }

        obj->Init(0, weak_from_this());
        m_EntityNameToIndex[std::string(name.begin(), name.end())] = m_Entities.size();

        m_Entities.emplace_back(obj);
        return std::dynamic_pointer_cast<T>(obj);
    }

private:
    entt::registry m_Registry;
    std::map<std::string, Actor> m_Actors;
    std::shared_ptr<ResourceManagerImpl> m_ResourceManager;

    std::unordered_map<MeshKey, std::shared_ptr<InstancedMesh>> m_MeshNameToInstancedMesh;
    std::vector<LightData> m_Lights;

    std::vector<std::shared_ptr<BaseEntity>> m_Entities;
    std::unordered_map<std::string, size_t> m_EntityNameToIndex;

    std::vector<std::shared_ptr<StaticMeshEntity>> m_StaticMeshEntity;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);

    Actor ConstructFromEntity(entt::entity entity) const
    {
        return Actor{std::const_pointer_cast<Level>(shared_from_this()), entt::handle{const_cast<entt::registry&>(m_Registry), entity}};
    }

    void Serialize(IArchive& archive);
    void SerializeActor(const Actor& actor, IArchive& archive, const std::string& name);
};

