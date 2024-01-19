#include "Input.h"

#include <GLFW/glfw3.h>

Input* Input::instance_ = nullptr;

Input::Input(GLFWwindow* window) :
    window_{window}
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    mouse_position_ = {(float)x, (float)y};
    instance_ = this;
}

bool Input::IsKeyPressedImpl(KeyCode key)
{
    int state = glfwGetKey(window_, key);
    return state == GLFW_PRESS;
}

bool Input::IsMouseButtonPressedImpl(MouseButton button)
{
    int state = glfwGetMouseButton(window_, button);
    return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePositionImpl() const
{
    return mouse_position_;
}

void Input::Update(const GlfwWindowData& window_data)
{
    mouse_position_ = window_data.mouse_position;
}

bool Input::IsKeyPressed(KeyCode key)
{
    return instance_->IsKeyPressedImpl(key);
}

bool Input::IsMouseButtonPressed(MouseButton button)
{
    return instance_->IsMouseButtonPressedImpl(button);
}

glm::vec2 Input::GetMousePosition()
{
    return instance_->GetMousePositionImpl();
}

float Input::GetMouseX()
{
    return instance_->GetMousePositionImpl().x;
}

float Input::GetMouseY()
{
    return instance_->GetMousePositionImpl().y;
}