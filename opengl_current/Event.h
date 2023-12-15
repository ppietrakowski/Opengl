#include <GLFW/glfw3.h>
#include <chrono>

#include <glm/glm.hpp>
#include <cstdint>

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
        std::uint32_t Width;
        std::uint32_t Height;
    };

    struct KeyEvent
    {
        std::int32_t Code;
        std::int32_t Scancode;
        bool AltClicked : 1;
        bool ControlClicked : 1;
        bool ShiftClicked : 1;
        bool SystemClicked : 1;
    };

    struct MouseMoveEvent
    {
        glm::vec2 MousePosition;
        glm::vec2 LastMousePosition;
    };

    struct MouseButtonEvent
    {
        std::int32_t Button;
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
        SizeEvent             Size;
        KeyEvent              Key;
        MouseMoveEvent        MouseMove;
        MouseButtonEvent      MouseButton;
        MouseWheelEvent       MouseWheel;
    };

    std::uint32_t MillisecondsSinceGameStart;

    Event();
};