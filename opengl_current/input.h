#pragma once

#include "glfw_window_data.h"

#include <glm/glm.hpp>

struct GLFWwindow;

class Input {
public:
    Input(GLFWwindow* window);
    ~Input() = default;

public:

    static bool IsKeyPressed(KeyCode key);

    static bool IsMouseButtonPressed(MouseButton button);
    static glm::vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();

    void Update(const GlfwWindowData& window_data);

private:
    static Input* instance_;

private:
    GLFWwindow* window_;
    glm::vec2 mouse_position_;

private:
    bool IsKeyPressedImpl(KeyCode key);
    bool IsMouseButtonPressedImpl(MouseButton button);
    glm::vec2 GetMousePositionImpl() const;
};

