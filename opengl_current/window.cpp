#include "window.h"
#include "glfw_window.h"

std::unique_ptr<Window> Window::Create(const WindowSettings& windowSettings)
{
    return std::make_unique<GlfwWindow>(windowSettings);
}
