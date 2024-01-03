#include "glfw_input.h"

GlfwInput::GlfwInput(GLFWwindow* window) :
    Window{window}
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    MousePosition = {(float)x, (float)y};
}

bool GlfwInput::IsKeyPressedImpl(KeyCode key)
{
    int32_t state = glfwGetKey(Window, static_cast<int32_t>(key));
    return state == GLFW_PRESS;
}

bool GlfwInput::IsMouseButtonPressedImpl(MouseButton button)
{
    int32_t state = glfwGetMouseButton(Window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

glm::vec2 GlfwInput::GetMousePositionImpl()
{
    return MousePosition;
}

void GlfwInput::Update(const GlfwWindowData& window_data)
{
    MousePosition = window_data.MousePosition;
}
