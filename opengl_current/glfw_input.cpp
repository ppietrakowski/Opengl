#include "glfw_input.h"

GlfwInput::GlfwInput(GLFWwindow* window) :
    window_{ window } {}

bool GlfwInput::IsKeyPressedImpl(KeyCode key) {
    int32_t state = glfwGetKey(window_, static_cast<int32_t>(key));
    return state == GLFW_PRESS;
}

bool GlfwInput::IsMouseButtonPressedImpl(MouseCode button) {
    int32_t state = glfwGetMouseButton(window_, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

glm::vec2 GlfwInput::GetMousePositionImpl() {
    return mouse_position;
}

void GlfwInput::Update(const WindowData& window_data) {
    mouse_position = window_data.mouse_position;
}
