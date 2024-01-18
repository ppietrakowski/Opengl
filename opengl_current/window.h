#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <memory>

#include "event.h"
#include "graphics_context.h"
#include "glfw_window_data.h"

struct GLFWwindow;

struct WindowSettings
{
    std::int32_t width;
    std::int32_t height;
    std::string title;
};

class Input;

class Window
{
public:
    Window(const WindowSettings& settings);
    ~Window();

public:

    void Update();

    std::int32_t GetWidth() const;
    std::int32_t GetHeight() const;

    glm::ivec2 GetWindowPosition() const;

    glm::vec2 GetMousePosition() const;
    glm::vec2 GetLastMousePosition() const;

    bool IsOpen() const;

    void SetEventCallback(const EventCallback& callback);
    void EnableVSync();
    void DisableVSync();
    bool IsVSyncEnabled() const;

    void* GetWindowNativeHandle() const;
    GraphicsContext* GetContext() const;

    void Close();
    void SetMouseVisible(bool mouse_visible);

private:
    GLFWwindow* window_;
    Input* input_;
    GraphicsContext* graphics_context_;
    GlfwWindowData window_data_;

private:
    void BindWindowCallbacks();
};

