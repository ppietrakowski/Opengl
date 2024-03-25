#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <any>

#include "GraphicsContext.hpp"
#include "GlfwWindowData.hpp"
#include "Input.hpp"

struct WindowSettings
{
    int Width;
    int Height;
    std::string Title;
};

class Input;

class Window
{
public:
    struct WindowDataImpl;
    friend struct Window::WindowDataImpl;

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

    std::any GetNativeWindow() const
    {
        return m_Window;
    }

    void SetWindowMessageHandler(std::shared_ptr<IWindowMessageHandler> windowMessageHandler);

private:
    GLFWwindow* m_Window;
    Input m_Input;
    GraphicsContext m_GraphicsContext;
    GlfwWindowData m_WindowData;
    std::unique_ptr<WindowDataImpl> m_WindowDataReal;
    
private:
    void BindWindowCallbacks();
    void ExecuteKeyEvent(KeyCode::Index keyCode, bool bPressed);
};

