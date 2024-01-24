#pragma once

#include <string>

class LevelInterface
{
public:
    virtual ~LevelInterface() = default;

    virtual void RemoveActor(const std::string& actorName) = 0;
    virtual void NotifyActorNameChanged(const std::string& oldName, const std::string& newName) = 0;
};