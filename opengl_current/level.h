#pragma once

#include "actor.h"
#include "duration.h"

#include "instanced_mesh.h"

class ResourceManagerImpl;

class Level
{
public:
    Level();
    ~Level();

    Actor CreateActor(const std::string& name);
    Actor FindActor(const std::string& name) const;
    std::vector<Actor> FindActorsWithTag(const std::string& tag) const;

    void RemoveActor(const std::string& name);

    void StartupLevel();
    void BroadcastUpdate(Duration duration);
    void BroadcastRender(Duration duration);

    auto begin()
    {
        return actors_.begin();
    }

    auto end()
    {
        return actors_.end();
    }

    template <typename ...Args>
    auto View()
    {
        return registry_.view<Args...>();
    }

    void AddNewStaticMesh(const std::string& mesh_name, const Transform& transform);

private:
    entt::registry registry_;
    std::map<std::string, Actor> actors_;
    std::shared_ptr<ResourceManagerImpl> resource_manager_;
    size_t num_frames_{0};

    std::unordered_map<std::string, std::shared_ptr<InstancedMesh>> instanced_mesh_;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);

    Actor ConstructFromEntity(entt::entity entity) const
    {
        Actor a{};
        a.entity_handle_ = entt::handle{const_cast<entt::registry&>(registry_), entity};
        a.home_level_ = const_cast<Level*>(this);
        return a;
    }
};

