#include "glfw_input.h"

GlfwInput::GlfwInput(GLFWwindow* window) :
    window_{window} {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    mouse_position_ = {(float)x, (float)y};
}

bool GlfwInput::IsKeyPressedImpl(KeyCode key) {
    std::int32_t state = glfwGetKey(window_, static_cast<std::int32_t>(key));
    return state == GLFW_PRESS;
}

bool GlfwInput::IsMouseButtonPressedImpl(MouseButton button) {
    std::int32_t state = glfwGetMouseButton(window_, static_cast<std::int32_t>(button));
    return state == GLFW_PRESS;
}

glm::vec2 GlfwInput::GetMousePositionImpl() {
    return mouse_position_;
}

void GlfwInput::Update(const GlfwWindowData& window_data) {
    mouse_position_ = window_data.mouse_position;
}
