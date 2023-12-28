#include "opengl_graphics_context.h"
#include "glfw_window.h"
#include "error_macros.h"
#include "logging.h"


GlfwWindow::GlfwWindow(const WindowSettings& settings)
{
    glfwSetErrorCallback([](int code, const char* description) {
        ELOG_ERROR(LOG_CORE, "Glfw error %i : %s", code, description);
    });

    // initialize glfw and create window with opengl 4.3 context
    CRASH_EXPECTED_TRUE(glfwInit());

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
    glfwGetWindowSize(m_Window, &m_WindowData.WindowSize.x, &m_WindowData.WindowSize.y);

    BindWindowCallbacks();
    m_GraphicsContext = new OpenGlGraphicsContext(m_Window);
    m_Input = new GlfwInput(m_Window);
}

GlfwWindow::~GlfwWindow()
{
    delete m_Input;
    delete m_GraphicsContext;
    glfwDestroyWindow(m_Window);
}

void GlfwWindow::Update()
{
    glfwPollEvents();
    m_GraphicsContext->SwapBuffers();
    m_Input->Update(m_WindowData);
}

uint32_t GlfwWindow::GetWidth() const
{
    return m_WindowData.WindowSize.x;
}

uint32_t GlfwWindow::GetHeight() const
{
    return m_WindowData.WindowSize.y;
}

glm::ivec2 GlfwWindow::GetWindowPosition() const
{
    return m_WindowData.WindowPosition;
}

glm::vec2 GlfwWindow::GetMousePosition() const
{
    return m_WindowData.MousePosition;
}

glm::vec2 GlfwWindow::GetLastMousePosition() const
{
    return m_WindowData.LastMousePosition;
}

bool GlfwWindow::IsOpen() const
{
    return m_WindowData.bGameRunning;
}

void GlfwWindow::SetEventCallback(const EventCallback& callback)
{
    m_WindowData.Callback = callback;
}

void GlfwWindow::EnableVSync()
{
    m_WindowData.bVsyncEnabled = true;
    m_GraphicsContext->SetVsync(true);
}

void GlfwWindow::DisableVSync()
{
    m_WindowData.bVsyncEnabled = false;
    m_GraphicsContext->SetVsync(false);
}

bool GlfwWindow::IsVSyncEnabled() const
{
    return m_WindowData.bVsyncEnabled;
}

void* GlfwWindow::GetWindowNativeHandle() const
{
    return m_Window;
}

IGraphicsContext* GlfwWindow::GetContext() const
{
    return m_GraphicsContext;
}

void GlfwWindow::Close()
{
    glfwSetWindowShouldClose(m_Window, GL_TRUE);
    m_WindowData.bGameRunning = false;
}

void GlfwWindow::SetMouseVisible(bool bMouseVisible)
{
    if (bMouseVisible)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void GlfwWindow::BindWindowCallbacks()
{
    glfwSetWindowUserPointer(m_Window, &m_WindowData);

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        windowData->LastMousePosition = windowData->MousePosition;
        windowData->MousePosition = glm::vec2{xpos, ypos};

        if (windowData->Callback)
        {
            Event evt{};
            evt.Type = EventType::kMouseMoved;
            evt.MouseMove.MousePosition = windowData->MousePosition;
            evt.MouseMove.LastMousePosition = windowData->LastMousePosition;
            windowData->Callback(evt);
        }
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::kKeyPressed : EventType::kKeyReleased;
            event.Key = {(KeyCode)key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER)};

            windowData->Callback(event);
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS) ? EventType::kMouseButtonPressed : EventType::kMouseButtonReleased;
            event.MouseButtonState = {(MouseButton)button, windowData->MousePosition};
            windowData->Callback(event);
        }
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (focused == GL_TRUE) ? EventType::kGainedFocus : EventType::kLostFocus;
            windowData->Callback(event);
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = EventType::kMouseWheelScrolled;
            event.MouseWheel.Delta = {xoffset, yoffset};
            windowData->Callback(event);
        }
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        windowData->bGameRunning = false;
    });
}
