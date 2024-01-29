#pragma once

#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdint>
#include <glm/glm.hpp>

using MouseButton = int32_t;
using KeyCode = int32_t;

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
    WindowResized,
    Count
};

struct Event
{
    struct SizeEvent
    {
        int32_t Width;
        int32_t Height;
    };

    struct KeyEvent
    {
        KeyCode Key;
        int32_t Scancode;
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
        MouseButtonEvent MouseButtonEvt;
        MouseWheelEvent MouseWheel;
    };

    std::chrono::milliseconds Timestamp;
    Event();
};