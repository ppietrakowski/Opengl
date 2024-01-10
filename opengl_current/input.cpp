#include "Input.h"

Input* Input::s_Instance = nullptr;

Input::Input()
{
    s_Instance = this;
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
