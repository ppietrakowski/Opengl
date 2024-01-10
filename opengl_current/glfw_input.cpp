#include "glfw_input.h"

GlfwInput::GlfwInput(GLFWwindow* window) :
    m_Window{window}
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    m_MousePosition = {(float)x, (float)y};
}

bool GlfwInput::IsKeyPressedImpl(KeyCode key)
{
    std::int32_t state = glfwGetKey(m_Window, static_cast<std::int32_t>(key));
    return state == GLFW_PRESS;
}

bool GlfwInput::IsMouseButtonPressedImpl(MouseButton button)
{
    std::int32_t state = glfwGetMouseButton(m_Window, static_cast<std::int32_t>(button));
    return state == GLFW_PRESS;
}

glm::vec2 GlfwInput::GetMousePositionImpl()
{
    return m_MousePosition;
}

void GlfwInput::Update(const GlfwWindowData& windowData)
{
    m_MousePosition = windowData.MousePosition;
}
