#include "window.h"
#include "glfw_window.h"

std::unique_ptr<Window> Window::Create(const WindowSettings& window_settings) {
    return std::make_unique<GlfwWindow>(window_settings);
}
