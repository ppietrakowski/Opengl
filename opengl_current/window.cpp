#include "window.h"
#include "glfw_window.h"

std::unique_ptr<IWindow> IWindow::Create(const WindowSettings& windowSettings)
{
    return std::make_unique<GlfwWindow>(windowSettings);
}
