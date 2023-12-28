#pragma once

#include "window.h"
#include <glm/glm.hpp>
#include <functional>

struct GlfwWindowData
{
    EventCallback Callback;
    bool bMouseVisible : 1{ true };
    glm::ivec2 WindowPosition{0, 0};
    glm::ivec2 WindowSize{0, 0};
    glm::vec2 MousePosition{0, 0};
    glm::vec2 LastMousePosition{0, 0};
    bool bGameRunning : 1{ true };
    bool bVsyncEnabled : 1{true};
};
