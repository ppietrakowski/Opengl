#include <GLFW/glfw3.h>
#include <chrono>

#include <glm/glm.hpp>

enum class EEventType
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
        unsigned int  Width;
        unsigned int  Height;
    };

    struct KeyEvent
    {
        int Code;
        int Scancode;
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
        int Button;
        glm::vec2 MousePosition;
    };

    struct MouseWheelEvent
    {
        glm::vec2 Delta;
    };

    // Member data
    EEventType Type;

    union
    {
        SizeEvent             Size;
        KeyEvent              Key;
        MouseMoveEvent        MouseMove;
        MouseButtonEvent      MouseButton;
        MouseWheelEvent       MouseWheel;
    };

    unsigned int  MillisecondsSinceGameStart;

    Event();
};