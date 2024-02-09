#pragma once

#include "Viewport.hpp"

#include <string>
#include <glm/glm.hpp>

class LevelInterface
{
public:
    virtual ~LevelInterface() = default;

    virtual void RemoveActor(const std::string& actorName) = 0;
    virtual void NotifyActorNameChanged(const std::string& oldName, const std::string& newName) = 0;

    static glm::vec3 ProjectWorldToScreen(glm::vec3 world);
    static glm::vec3 ProjectScreenToWorld(glm::vec3 screen);

    static void BeginScene(const glm::mat4& projection, const glm::mat4& view, const Viewport& viewport);

private:

    static inline glm::mat4 s_Projection{1.0f};
    static inline glm::mat4 s_View{1.0f};
    static inline Viewport s_Viewport{};
};