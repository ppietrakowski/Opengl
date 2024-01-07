#include "Input.h"

Input* Input::input_instance_ = nullptr;

Input::Input()
{
    input_instance_ = this;
}

bool Input::IsKeyPressed(KeyCode key)
{
    return input_instance_->IsKeyPressedImpl(key);
}

bool Input::IsMouseButtonPressed(MouseButton button)
{
    return input_instance_->IsMouseButtonPressedImpl(button);
}

glm::vec2 Input::GetMousePosition()
{
    return input_instance_->GetMousePositionImpl();
}

float Input::GetMouseX()
{
    return input_instance_->GetMousePositionImpl().x;
}

float Input::GetMouseY()
{
    return input_instance_->GetMousePositionImpl().y;
}
