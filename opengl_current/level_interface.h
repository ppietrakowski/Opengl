#pragma once

#include <string>

class LevelInterface {
public:
    virtual ~LevelInterface() = default;

    virtual void RemoveActor(const std::string& actor_name) = 0;
    virtual void NotifyActorNameChanged(const std::string& old_name, const std::string& new_name) = 0;
};