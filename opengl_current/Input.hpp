#pragma once

#include "GlfwWindowData.hpp"

#include <glm/glm.hpp>

struct GLFWwindow;

class Input
{
public:
    Input(GLFWwindow* window);
    ~Input() = default;

public:

    static bool IsKeyPressed(KeyCode::Index key);

    static bool IsMouseButtonPressed(MouseButton button);
    static glm::vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();

    void Update(const GlfwWindowData& windowData);

private:
    static Input* s_Instance;

private:
    GLFWwindow* m_Window;
    glm::vec2 m_MousePosition;

private:
    bool IsKeyPressedImpl(KeyCode::Index key);
    bool IsMouseButtonPressedImpl(MouseButton button);
    glm::vec2 GetMousePositionImpl() const;
};

