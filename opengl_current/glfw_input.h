#pragma once

#include "input.h"
#include "glfw_window_data.h"

#include <GLFW/glfw3.h>

class GlfwInput : public Input {
public:
    GlfwInput(GLFWwindow* window);

    bool IsKeyPressedImpl(KeyCode key) override;
    bool IsMouseButtonPressedImpl(MouseCode button) override;
    glm::vec2 GetMousePositionImpl() override;

    void Update(const WindowData& window_data);

private:
    GLFWwindow* window_;
    glm::vec2 mouse_position_;
};
