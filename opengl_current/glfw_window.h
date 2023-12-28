#pragma once

#include "window.h"
#include "glfw_input.h"
#include "glfw_window_data.h"

#include <GLFW/glfw3.h>

class GlfwWindow : public IWindow
{
public:
    GlfwWindow(const WindowSettings& settings);
    ~GlfwWindow();

public:

    void Update() override;
    int32_t GetWidth() const override;
    int32_t GetHeight() const override;

    glm::ivec2 GetWindowPosition() const override;
    glm::vec2 GetMousePosition() const override;
    glm::vec2 GetLastMousePosition() const override;

    bool IsOpen() const override;
    void SetEventCallback(const EventCallback& callback) override;
    void EnableVSync() override;

    void DisableVSync() override;
    bool IsVSyncEnabled() const override;
    void* GetWindowNativeHandle() const override;
    IGraphicsContext* GetContext() const override;

    void Close() override;
    void SetMouseVisible(bool bMouseVisible) override;

private:
    GLFWwindow* m_Window;
    GlfwInput* m_Input;
    IGraphicsContext* m_GraphicsContext;
    GlfwWindowData m_WindowData;

private:
    void BindWindowCallbacks();
};

