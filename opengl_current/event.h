#pragma once

#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdint>
#include <glm/glm.hpp>

using MouseButton = int;
using KeyCode = int;

enum class EventType {
    kInvalid = 0,
    kLostFocus,
    kGainedFocus,
    kKeyPressed,
    kKeyReleased,
    kMouseWheelScrolled,
    kMouseButtonPressed,
    kMouseButtonReleased,
    kMouseMoved,
    kCount
};

struct Event {
    struct SizeEvent {
        int width;
        int height;
    };

    struct KeyEvent {
        KeyCode key;
        int scancode;
        bool alt_clicked : 1;
        bool control_clicked : 1;
        bool shift_clicked : 1;
        bool system_clicked : 1;
    };

    struct MouseMoveEvent {
        glm::vec2 mouse_position;
        glm::vec2 last_mouse_position;
    };

    struct MouseButtonEvent {
        MouseButton button;
        glm::vec2 mouse_position;
    };

    struct MouseWheelEvent {
        glm::vec2 delta;
    };

    // Member data
    EventType type;

    union {
        SizeEvent size;
        KeyEvent key;
        MouseMoveEvent mouse_move;
        MouseButtonEvent mouse_button;
        MouseWheelEvent mouse_wheel;
    };

    std::chrono::milliseconds timestamp;
    Event();
};