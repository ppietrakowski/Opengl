#pragma once

#include "Actor.h"
#include "duration.h"

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

private:
    entt::registry m_Registry;
    std::map<std::string, Actor> m_Actors;
    std::shared_ptr<ResourceManagerImpl> m_ResourceManager;

private:
    void UpdateSkeletalMeshesAnimation(Duration duration);

    Actor ConstructFromEntity(entt::entity entity) const
    {
        Actor a{};
        a.m_EntityHandle = entt::handle{const_cast<entt::registry&>(m_Registry), entity};
        a.m_HomeLevel = const_cast<Level*>(this);
        return a;
    }
};

