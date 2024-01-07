#pragma once

#include "Actor.h"
#include "duration.h"
#include "resouce_manager.h"

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

private:
    entt::registry registry_;
    std::unordered_map<std::string, Actor> actors_;
    std::shared_ptr<ResourceManagerImpl> resource_manager_;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);
};

