#pragma once

#include <chrono>
#include <cstdint>
#include <glm/glm.hpp>

#include "keys.h"

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
        uint32_t width;
        uint32_t height;
    };

    struct KeyEvent {
        KeyCode code;
        int32_t scan_code;
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
        MouseCode button;
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

    uint32_t milliseconds_since_game_start;
    Event();
};