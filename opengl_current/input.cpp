#include "Input.h"

Input* Input::instance_ = nullptr;

Input::Input() {
    instance_ = this;
}

bool Input::IsKeyPressed(KeyCode key) {
    return instance_->IsKeyPressedImpl(key);
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    return instance_->IsMouseButtonPressedImpl(button);
}

glm::vec2 Input::GetMousePosition() {
    return instance_->GetMousePositionImpl();
}

float Input::GetMouseX() {
    return instance_->GetMousePositionImpl().x;
}

float Input::GetMouseY() {
    return instance_->GetMousePositionImpl().y;
}
