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
    uint32_t Width;
    uint32_t Height;
    std::string Title;
};

class Input;

class Window
{
public:
    Window(const WindowSettings& settings);
    ~Window();

public:

    void Update();

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

    glm::ivec2 GetWindowPosition() const;

    glm::vec2 GetMousePosition() const;
    glm::vec2 GetLastMousePosition() const;

    bool IsOpen() const;

    void SetEventCallback(const EventCallback& callback);
    void EnableVSync();
    void DisableVSync();
    bool IsVSyncEnabled() const;

    void Close();
    void SetMouseVisible(bool bMouseVisible);

    bool IsMouseVisible() const
    {
        return m_WindowData.bMouseVisible;
    }

    void InitializeImGui();
    void DeinitializeImGui();

    void ImGuiBeginFrame();
    void ImGuiDrawFrame();
    void ImGuiUpdateViewport();

    void* GetNativeWindow() const
    {
        return m_Window;
    }

private:
    GLFWwindow* m_Window;
    Input* m_Input;
    GraphicsContext* m_GraphicsContext;
    GlfwWindowData m_WindowData;

private:
    void BindWindowCallbacks();
};

