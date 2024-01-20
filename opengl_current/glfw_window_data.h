#pragma once

#include "event.h"
#include <glm/glm.hpp>
#include <functional>

using EventCallback = std::function<void(const Event&)>;

struct GlfwWindowData {
    EventCallback event_callback;
    bool mouse_visible : 1{ true };
    glm::ivec2 window_position{0, 0};
    glm::uvec2 window_size{0, 0};
    glm::vec2 mouse_position{0, 0};
    glm::vec2 last_mouse_position{0, 0};
    bool game_running : 1{ true };
    bool vsync_enabled : 1{true};
};
