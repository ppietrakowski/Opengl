#include "Input.h"

#include <GLFW/glfw3.h>

Input* Input::s_Instance = nullptr;

Input::Input(GLFWwindow* window) :
    m_Window{window}
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    m_MousePosition = {(float)x, (float)y};
    s_Instance = this;
}

bool Input::IsKeyPressedImpl(KeyCode key)
{
    int state = glfwGetKey(m_Window, key);
    return state == GLFW_PRESS;
}

bool Input::IsMouseButtonPressedImpl(MouseButton button)
{
    int state = glfwGetMouseButton(m_Window, button);
    return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePositionImpl() const
{
    return m_MousePosition;
}

void Input::Update(const GlfwWindowData& windowData)
{
    m_MousePosition = windowData.MousePosition;
}

bool Input::IsKeyPressed(KeyCode key)
{
    return s_Instance->IsKeyPressedImpl(key);
}

bool Input::IsMouseButtonPressed(MouseButton button)
{
    return s_Instance->IsMouseButtonPressedImpl(button);
}

glm::vec2 Input::GetMousePosition()
{
    return s_Instance->GetMousePositionImpl();
}

float Input::GetMouseX()
{
    return s_Instance->GetMousePositionImpl().x;
}

float Input::GetMouseY()
{
    return s_Instance->GetMousePositionImpl().y;
}