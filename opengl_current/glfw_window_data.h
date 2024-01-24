#pragma once

#include "event.h"
#include <glm/glm.hpp>
#include <functional>

using EventCallback = std::function<void(const Event&)>;

struct GlfwWindowData
{
    EventCallback Callback;
    bool bMouseVisible : 1{ true };
    glm::ivec2 WindowPosition{0, 0};
    glm::uvec2 WindowSize{0, 0};
    glm::vec2 MousePosition{0, 0};
    glm::vec2 LastMousePosition{0, 0};
    bool bGameRunning : 1{ true };
    bool bVsyncEnabled : 1{true};
};
