#pragma once

#include <chrono>
#include <cstdint>
#include <glm/glm.hpp>

#include "keys.h"

enum class EventType
{
    Invalid = 0,
    LostFocus,
    GainedFocus,
    KeyPressed,
    KeyReleased,
    MouseWheelScrolled,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    Count
};

struct Event
{
    struct SizeEvent
    {
        std::int32_t Width;
        std::int32_t Height;
    };

    struct KeyEvent
    {
        KeyCode Key;
        std::int32_t Scancode;
        bool bAltClicked : 1;
        bool bControlClicked : 1;
        bool bShiftClicked : 1;
        bool bSystemClicked : 1;
    };

    struct MouseMoveEvent
    {
        glm::vec2 MousePosition;
        glm::vec2 LastMousePosition;
    };

    struct MouseButtonEvent
    {
        MouseButton Button;
        glm::vec2 MousePosition;
    };

    struct MouseWheelEvent
    {
        glm::vec2 Delta;
    };

    // Member data
    EventType Type;

    union
    {
        SizeEvent Size;
        KeyEvent Key;
        MouseMoveEvent MouseMove;
        MouseButtonEvent MouseButton;
        MouseWheelEvent MouseWheel;
    };

    std::chrono::milliseconds Timestamp;
    Event();
};