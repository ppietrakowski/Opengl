#pragma once

#include "Actor.hpp"
#include "Datapack.hpp"
#include <entt/entt.hpp>

class ActorComponent
{
    friend class Actor;

public:
    template <typename T>
    T& GetComponent()
    {
        return m_Actor.GetComponent<T>();
    }

    template <typename T>
    const T& GetComponent() const
    {
        return m_Actor.GetComponent<T>();
    }

    template <typename T, typename ...Args>
    void AddComponent(Args&& ...args)
    {
        m_Actor.AddComponent(std::forward<Args>(args)...);
    }

    template <typename T>
    void RemoveComponent()
    {
        m_Actor.RemoveComponent<T>();
    }

    virtual Datapack Archived() const;
    virtual void Unarchive(const Datapack& datapack);

private:
    Actor m_Actor;
};

