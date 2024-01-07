#pragma once

#include "window.h"
#include "glfw_input.h"
#include "glfw_window_data.h"

#include <GLFW/glfw3.h>

class GlfwWindow : public Window
{
public:
    GlfwWindow(const WindowSettings& settings);
    ~GlfwWindow();

public:

    void Update() override;
    std::int32_t GetWidth() const override;
    std::int32_t GetHeight() const override;

    glm::ivec2 GetWindowPosition() const override;
    glm::vec2 GetMousePosition() const override;
    glm::vec2 GetLastMousePosition() const override;

    bool IsOpen() const override;
    void SetEventCallback(const EventCallback& callback) override;
    void EnableVSync() override;

    void DisableVSync() override;
    bool IsVSyncEnabled() const override;
    void* GetWindowNativeHandle() const override;
    GraphicsContext* GetContext() const override;

    void Close() override;
    void SetMouseVisible(bool mouse_visible) override;

private:
    GLFWwindow* window_;
    GlfwInput* input_;
    GraphicsContext* graphics_context_;
    GlfwWindowData window_data_;

private:
    void BindWindowCallbacks();
};

