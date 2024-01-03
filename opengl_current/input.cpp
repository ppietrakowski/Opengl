#include "Input.h"

Input* Input::Instance = nullptr;

Input::Input()
{
    Instance = this;
}

bool Input::IsKeyPressed(KeyCode key)
{
    return Instance->IsKeyPressedImpl(key);
}

bool Input::IsMouseButtonPressed(MouseButton button)
{
    return Instance->IsMouseButtonPressedImpl(button);
}

glm::vec2 Input::GetMousePosition()
{
    return Instance->GetMousePositionImpl();
}

float Input::GetMouseX()
{
    return Instance->GetMousePositionImpl().x;
}

float Input::GetMouseY()
{
    return Instance->GetMousePositionImpl().y;
}
