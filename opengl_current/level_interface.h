#pragma once

#include <string>

class LevelInterface {
public:
    virtual ~LevelInterface() = default;

    virtual void RemoveActor(const std::string& actor_name) = 0;
};