#pragma once

#include "GlfwWindowData.hpp"

#include <glm/glm.hpp>
#include <array>

struct GLFWwindow;

struct KeyState
{
    bool bCurrentState : 1 = false;
    bool bPrevState : 1 = false;
};

constexpr int MaxNumKeys = KeyCode::LastKey + 1;

class Input
{
    friend class Window;
public:
    Input() = default;
    ~Input() = default;

private:
    void Initialize(GLFWwindow* window);

public:

    static bool IsKeyPressed(KeyCode::Index key);

    static bool IsMouseButtonPressed(MouseButton button);
    static glm::vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();

    void Update(const GlfwWindowData& windowData);
    static bool IsKeyToggled(KeyCode::Index key);

private:
    static Input* s_Instance;

private:
    GLFWwindow* m_Window;
    glm::vec2 m_MousePosition;

    std::array<KeyState, MaxNumKeys> m_KeysMap;
    std::vector<KeyCode::Index> m_ChangedKeyMaps;

private:
    bool IsKeyPressedImpl(KeyCode::Index key);
    bool IsMouseButtonPressedImpl(MouseButton button);
    glm::vec2 GetMousePositionImpl() const;

    void OnKeyEvent(KeyCode::Index key, bool bPressed);
};

