#pragma once

#include "input.h"
#include "glfw_window_data.h"

#include <GLFW/glfw3.h>

class GlfwInput : public Input
{
public:
    GlfwInput(GLFWwindow* window);

    bool IsKeyPressedImpl(KeyCode key) override;
    bool IsMouseButtonPressedImpl(MouseButton button) override;
    glm::vec2 GetMousePositionImpl() override;

    void Update(const GlfwWindowData& windowData);

private:
    GLFWwindow* m_Window;
    glm::vec2 m_MousePosition;
};

