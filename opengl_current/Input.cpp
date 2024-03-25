#include "Input.hpp"

#include <GLFW/glfw3.h>

Input* Input::s_Instance = nullptr;

bool Input::IsKeyPressedImpl(KeyCode::Index key)
{
    int state = glfwGetKey(m_Window, key);
    return state == GLFW_PRESS;
}

bool Input::IsMouseButtonPressedImpl(MouseButton button)
{
    int state = glfwGetMouseButton(m_Window, static_cast<int>(button));
    return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePositionImpl() const
{
    return m_MousePosition;
}

void Input::OnKeyEvent(KeyCode::Index key, bool bPressed)
{
    m_KeysMap[key].bPrevState = !bPressed;
    m_KeysMap[key].bCurrentState = bPressed;

    m_ChangedKeyMaps.push_back(key);
}

void Input::Update(const GlfwWindowData& windowData)
{
    m_MousePosition = windowData.MousePosition;

    for (KeyCode::Index key : m_ChangedKeyMaps)
    {
        m_KeysMap[key].bPrevState = m_KeysMap[key].bCurrentState;
    }

    m_ChangedKeyMaps.clear();
}

bool Input::IsKeyToggled(KeyCode::Index key)
{
    KeyState state = s_Instance->m_KeysMap[key];
    return state.bCurrentState != state.bPrevState;
}

void Input::Initialize(GLFWwindow* window)
{
    m_Window = window;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    m_MousePosition = {(float)x, (float)y};
    s_Instance = this;

    for (int i = 0; i < MaxNumKeys; ++i)
    {
        m_KeysMap[i].bCurrentState = glfwGetKey(window, i);
    }

    m_ChangedKeyMaps.reserve(MaxNumKeys);
}

bool Input::IsKeyPressed(KeyCode::Index key)
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