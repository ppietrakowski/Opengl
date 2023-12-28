#pragma once

#include <chrono>
#include <cstdint>
#include <glm/glm.hpp>

#include "keys.h"

enum class EventType
{
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

struct Event
{
    struct SizeEvent
    {
        int32_t Width;
        int32_t Height;
    };

    struct KeyEvent
    {
        KeyCode Code;
        int32_t ScanCode;
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
        MouseButtonEvent MouseButtonState;
        MouseWheelEvent MouseWheel;
    };

    int32_t MillisecondsSinceGameStart;
    Event();
};