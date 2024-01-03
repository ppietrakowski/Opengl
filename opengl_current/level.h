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
    entt::registry Registry;
    std::unordered_map<std::string, Actor> Actors;
    std::shared_ptr<ResourceManagerImpl> ResourceManagerInstance;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);
};

