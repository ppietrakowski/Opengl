#include "Window.hpp"

#include "ErrorMacros.hpp"
#include "Input.hpp"
#include "Logging.hpp"
#include "imgui.h"
#include "Imgui/imgui_impl_glfw.h"

#include <GLFW/glfw3.h>

struct Window::WindowDataImpl
{
    Window* WindowInstance = NULL;
    GlfwWindowData* WindowData = NULL;

    void ExecuteInputOnEvent(KeyCode::Index keyCode, bool bPressed)
    {
        WindowInstance->ExecuteKeyEvent(keyCode, bPressed);
    }
};

Window::Window(const WindowSettings& settings)
{
#if defined(DEBUG) || defined(_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(settings.Width, settings.Height, settings.Title.c_str(), nullptr, nullptr);
    CRASH_EXPECTED_NOT_NULL(m_Window);

    m_WindowDataReal = std::make_unique<WindowDataImpl>();
    m_WindowDataReal->WindowData = &m_WindowData;
    m_WindowDataReal->WindowInstance = this;

    // fill window data
    glm::dvec2 mousePosition;
    glfwGetCursorPos(m_Window, &mousePosition.x, &mousePosition.y);
    m_WindowData.MousePosition = mousePosition;
    m_WindowData.LastMousePosition = m_WindowData.MousePosition;

    glfwGetWindowPos(m_Window, &m_WindowData.WindowPosition.x, &m_WindowData.WindowPosition.y);

    glm::ivec2 windowSize{0, 0};
    glfwGetWindowSize(m_Window, &windowSize.x, &windowSize.y);
    m_WindowData.WindowSize = windowSize;

    BindWindowCallbacks();
    m_GraphicsContext.Initialize(*this);
    m_Input.Initialize(m_Window);
}

Window::~Window()
{
    glfwDestroyWindow(m_Window);
}

void Window::Update()
{
    glfwPollEvents();
    m_GraphicsContext.SwapBuffers();
    m_Input.Update(m_WindowData);
}

uint32_t Window::GetWidth() const
{
    return m_WindowData.WindowSize.x;
}

uint32_t Window::GetHeight() const
{
    return m_WindowData.WindowSize.y;
}

glm::ivec2 Window::GetWindowPosition() const
{
    return m_WindowData.WindowPosition;
}

glm::vec2 Window::GetMousePosition() const
{
    return m_WindowData.MousePosition;
}

glm::vec2 Window::GetLastMousePosition() const
{
    return m_WindowData.LastMousePosition;
}

bool Window::IsOpen() const
{
    return m_WindowData.bGameRunning;
}

void Window::EnableVSync()
{
    m_WindowData.bVsyncEnabled = true;
    m_GraphicsContext.SetVsync(true);
}

void Window::DisableVSync()
{
    m_WindowData.bVsyncEnabled = false;
    m_GraphicsContext.SetVsync(false);
}

bool Window::IsVSyncEnabled() const
{
    return m_WindowData.bVsyncEnabled;
}

void Window::Close()
{
    glfwSetWindowShouldClose(m_Window, GL_TRUE);
    m_WindowData.bGameRunning = false;
}

void Window::SetMouseVisible(bool bMouseVisible)
{
    if (bMouseVisible)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    m_WindowData.bMouseVisible = bMouseVisible;
}

void Window::InitializeImGui()
{
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    m_GraphicsContext.InitializeForImGui();
}

void Window::DeinitializeImGui()
{
    m_GraphicsContext.DeinitializeImGui();
    ImGui_ImplGlfw_Shutdown();
}

void Window::ImGuiBeginFrame()
{
    m_GraphicsContext.ImGuiBeginFrame();
    ImGui_ImplGlfw_NewFrame();
}

void Window::ImGuiDrawFrame()
{
    m_GraphicsContext.ImGuiDrawFrame();
}

void Window::ImGuiUpdateViewport()
{
    ImGuiIO& io = ImGui::GetIO();

    // update viewport if enabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* currentContextWindow = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(currentContextWindow);
    }
}

void Window::SetWindowMessageHandler(std::shared_ptr<IWindowMessageHandler> windowMessageHandler)
{
    m_WindowData.WindowMessageHandler = windowMessageHandler;
}

inline static MouseButton GlfwMouseCodeToMouseButton(int code)
{
    switch (code)
    {
    case GLFW_MOUSE_BUTTON_RIGHT:
        return MouseButton::Right;
    case GLFW_MOUSE_BUTTON_LEFT:
        return MouseButton::Left;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        return MouseButton::Middle;
    }

    return MouseButton::Invalid;
}

void Window::BindWindowCallbacks()
{
    glfwSetWindowUserPointer(m_Window, m_WindowDataReal.get());

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
    {
        auto wdata = reinterpret_cast<Window::WindowDataImpl*>(glfwGetWindowUserPointer(window));
        GlfwWindowData* windowData = wdata->WindowData;

        windowData->LastMousePosition = windowData->MousePosition;
        windowData->MousePosition = glm::vec2{xpos, ypos};

        std::shared_ptr<IWindowMessageHandler> messageHandler = windowData->GetWindowMessageHandler();
        messageHandler->OnMouseMove(windowData->MousePosition);
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto wdata = reinterpret_cast<Window::WindowDataImpl*>(glfwGetWindowUserPointer(window));
        GlfwWindowData* windowData = wdata->WindowData;

        std::shared_ptr<IWindowMessageHandler> messageHandler = windowData->GetWindowMessageHandler();

        KeyCode::Index keyCode = static_cast<KeyCode::Index>(key);

        switch (action)
        {
        case GLFW_PRESS:
            messageHandler->OnKeyDown(keyCode);
            wdata->ExecuteInputOnEvent(keyCode, true);
            break;
        case GLFW_REPEAT:
            messageHandler->OnKeyRepeat(keyCode);
            break;
        case GLFW_RELEASE:
            messageHandler->OnKeyUp(static_cast<KeyCode::Index>(key));
            wdata->ExecuteInputOnEvent(keyCode, false);
            break;
        }

    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
    {
        auto wdata = reinterpret_cast<Window::WindowDataImpl*>(glfwGetWindowUserPointer(window));
        GlfwWindowData* windowData = wdata->WindowData;
        std::shared_ptr<IWindowMessageHandler> messageHandler = windowData->GetWindowMessageHandler();

        MouseButton mouseButton = GlfwMouseCodeToMouseButton(button);

        if (button == GLFW_PRESS)
        {
            messageHandler->OnMouseButtonDown(mouseButton, windowData->MousePosition);
        }
        else if (button == GLFW_RELEASE)
        {
            messageHandler->OnMouseButtonUp(mouseButton, windowData->MousePosition);
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        auto wdata = reinterpret_cast<Window::WindowDataImpl*>(glfwGetWindowUserPointer(window));
        GlfwWindowData* windowData = wdata->WindowData;
        std::shared_ptr<IWindowMessageHandler> messageHandler = windowData->GetWindowMessageHandler();

        messageHandler->OnMouseWheel(static_cast<float>(xoffset));
    });

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
    {
        auto wdata = reinterpret_cast<Window::WindowDataImpl*>(glfwGetWindowUserPointer(window));
        GlfwWindowData* windowData = wdata->WindowData;
        std::shared_ptr<IWindowMessageHandler> messageHandler = windowData->GetWindowMessageHandler();

        messageHandler->OnSizeChanged(glm::uvec2(width, height));
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
    {
        auto wdata = reinterpret_cast<Window::WindowDataImpl*>(glfwGetWindowUserPointer(window));
        GlfwWindowData* windowData = wdata->WindowData;
        windowData->bGameRunning = false;

        std::shared_ptr<IWindowMessageHandler> messageHandler = windowData->GetWindowMessageHandler();
        messageHandler->OnWindowClose();
    });
}

void Window::ExecuteKeyEvent(KeyCode::Index keyCode, bool bPressed)
{
}
