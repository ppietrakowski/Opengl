#pragma once

#include "WindowMessageHandler.hpp"
#include "Core.hpp"
#include "ErrorMacros.hpp"
#include <glm/glm.hpp>
#include <functional>

struct GlfwWindowData
{
    bool bMouseVisible : 1{ true };
    glm::ivec2 WindowPosition{0, 0};
    glm::uvec2 WindowSize{0, 0};
    glm::vec2 MousePosition{0, 0};
    glm::vec2 LastMousePosition{0, 0};
    bool bGameRunning : 1{ true };
    bool bVsyncEnabled : 1{true};
    std::weak_ptr<IWindowMessageHandler> WindowMessageHandler;

    std::shared_ptr<IWindowMessageHandler> GetWindowMessageHandler() const
    {
        auto ptr = WindowMessageHandler.lock();
        ASSERT(ptr);
        return ptr;
    }
};
