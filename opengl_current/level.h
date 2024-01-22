#pragma once

#include "actor.h"
#include "duration.h"

#include "instanced_mesh.h"

class ResourceManagerImpl;

class Level : public LevelInterface {
public:
    Level();
    ~Level();

    Actor CreateActor(const std::string& name);
    Actor FindActor(const std::string& name) const;
    std::vector<Actor> FindActorsWithTag(const std::string& tag) const;

    void RemoveActor(const std::string& name) override;
    void NotifyActorNameChanged(const std::string& old_name, const std::string& new_name) override;

    void StartupLevel();
    void BroadcastUpdate(Duration duration);
    void BroadcastRender(Duration duration);

    auto begin() {
        return actors_.begin();
    }

    auto end() {
        return actors_.end();
    }

    template <typename ...Args>
    auto View() {
        return registry_.view<Args...>();
    }

    template <typename ...Args>
    const auto View() const {
        return registry_.view<Args...>();
    }

    void AddNewStaticMesh(const std::string& mesh_name, const Transform& transform);

    bool TryFindActor(const std::string& name, Actor& out_actor);

    const CameraComponent& FindCameraComponent() const;

private:
    entt::registry registry_;
    std::map<std::string, Actor> actors_;
    std::shared_ptr<ResourceManagerImpl> resource_manager_;

    std::unordered_map<std::string, std::shared_ptr<InstancedMesh>> instanced_mesh_;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);

    Actor ConstructFromEntity(entt::entity entity) const {
        return Actor{const_cast<Level*>(this), entt::handle{const_cast<entt::registry&>(registry_), entity}};
    }
};

