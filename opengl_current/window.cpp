#include "window.h"

#include "error_macros.h"
#include "input.h"
#include "logging.h"
#include "imgui.h"
#include "Imgui/imgui_impl_glfw.h"

#include <GLFW/glfw3.h>


Window::Window(const WindowSettings& settings)
{
    glfwSetErrorCallback([](std::int32_t code, const char* description)
    {
        ELOG_ERROR(LOG_CORE, "Glfw error %i : %s", code, description);
    });

    // initialize glfw and create window with opengl 4.3 context
    CRASH_EXPECTED_TRUE(glfwInit());

#if defined(DEBUG) || defined(_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(settings.Width, settings.Height, settings.Title.c_str(), nullptr, nullptr);
    CRASH_EXPECTED_NOT_NULL(m_Window);

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
    m_GraphicsContext = new GraphicsContext(*this);
    m_Input = new Input(m_Window);
}

Window::~Window()
{
    SafeDelete(m_GraphicsContext);
    SafeDelete(m_Input);
    glfwDestroyWindow(m_Window);
}

void Window::Update()
{
    glfwPollEvents();
    m_GraphicsContext->SwapBuffers();
    m_Input->Update(m_WindowData);
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

void Window::SetEventCallback(const EventCallback& callback)
{
    m_WindowData.Callback = callback;
}

void Window::EnableVSync()
{
    m_WindowData.bVsyncEnabled = true;
    m_GraphicsContext->SetVsync(true);
}

void Window::DisableVSync()
{
    m_WindowData.bVsyncEnabled = false;
    m_GraphicsContext->SetVsync(false);
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
    m_GraphicsContext->InitializeForImGui();
}

void Window::DeinitializeImGui()
{
    m_GraphicsContext->DeinitializeImGui();
    ImGui_ImplGlfw_Shutdown();
}

void Window::ImGuiBeginFrame()
{
    m_GraphicsContext->ImGuiBeginFrame();
    ImGui_ImplGlfw_NewFrame();
}

void Window::ImGuiDrawFrame()
{
    m_GraphicsContext->ImGuiDrawFrame();
}

void Window::ImGuiUpdateViewport()
{
    ImGuiIO& io = ImGui::GetIO();

    // update viewport if bEnabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* currentContextWindow = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(currentContextWindow);
    }
}

void Window::BindWindowCallbacks()
{
    glfwSetWindowUserPointer(m_Window, &m_WindowData);

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        windowData->LastMousePosition = windowData->MousePosition;
        windowData->MousePosition = glm::vec2{xpos, ypos};

        if (windowData->Callback)
        {
            Event evt{};
            evt.Type = EventType::MouseMoved;
            evt.MouseMove.MousePosition = windowData->MousePosition;
            evt.MouseMove.LastMousePosition = windowData->LastMousePosition;
            windowData->Callback(evt);
        }
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::KeyPressed : EventType::KeyReleased;
            event.Key = {(KeyCode)key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER)};

            windowData->Callback(event);
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS) ? EventType::MouseButtonPressed : EventType::MouseButtonReleased;
            event.MouseButtonEvt = {(MouseButton)button, windowData->MousePosition};
            windowData->Callback(event);
        }
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (focused == GL_TRUE) ? EventType::GainedFocus : EventType::LostFocus;
            windowData->Callback(event);
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = EventType::MouseWheelScrolled;
            event.MouseWheel.Delta = {xoffset, yoffset};
            windowData->Callback(event);
        }
    });

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        if (windowData->Callback)
        {
            Event event{};
            event.Type = EventType::WindowResized;
            event.Size.Width = width;
            event.Size.Height = height;
            windowData->Callback(event);
        }
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
    {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        windowData->bGameRunning = false;
    });
}
