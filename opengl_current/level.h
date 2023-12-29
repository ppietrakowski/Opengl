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
    entt::registry m_Registry;
    std::unordered_map<std::string, Actor> m_Actors;
    std::shared_ptr<ResourceManagerImpl> m_ResourceManager;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);
};

