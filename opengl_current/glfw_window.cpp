#include "opengl_graphics_context.h"
#include "glfw_window.h"
#include "error_macros.h"
#include "logging.h"


GlfwWindow::GlfwWindow(const WindowSettings& settings)
{
    glfwSetErrorCallback([](int32_t code, const char* description) {
        ELOG_ERROR(LOG_CORE, "Glfw error %i : %s", code, description);
    });

    // initialize glfw and create window with opengl 4.3 context
    CRASH_EXPECTED_TRUE(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Window = glfwCreateWindow(settings.Width, settings.Height, settings.Title.c_str(), nullptr, nullptr);
    CRASH_EXPECTED_NOT_NULL(Window);

    // fill window data
    glm::dvec2 mousePosition;
    glfwGetCursorPos(Window, &mousePosition.x, &mousePosition.y);
    WindowData.MousePosition = mousePosition;
    WindowData.LastMousePosition = WindowData.MousePosition;

    glfwGetWindowPos(Window, &WindowData.WindowPosition.x, &WindowData.WindowPosition.y);
    glfwGetWindowSize(Window, &WindowData.WindowSize.x, &WindowData.WindowSize.y);

    BindWindowCallbacks();
    GraphicsContext = new OpenGlGraphicsContext(Window);
    Input = new GlfwInput(Window);
}

GlfwWindow::~GlfwWindow()
{
    delete Input;
    delete GraphicsContext;
    glfwDestroyWindow(Window);
}

void GlfwWindow::Update()
{
    glfwPollEvents();
    GraphicsContext->SwapBuffers();
    Input->Update(WindowData);
}

int32_t GlfwWindow::GetWidth() const
{
    return WindowData.WindowSize.x;
}

int32_t GlfwWindow::GetHeight() const
{
    return WindowData.WindowSize.y;
}

glm::ivec2 GlfwWindow::GetWindowPosition() const
{
    return WindowData.WindowPosition;
}

glm::vec2 GlfwWindow::GetMousePosition() const
{
    return WindowData.MousePosition;
}

glm::vec2 GlfwWindow::GetLastMousePosition() const
{
    return WindowData.LastMousePosition;
}

bool GlfwWindow::IsOpen() const
{
    return WindowData.bGameRunning;
}

void GlfwWindow::SetEventCallback(const EventCallback& callback)
{
    WindowData.Callback = callback;
}

void GlfwWindow::EnableVSync()
{
    WindowData.bVsyncEnabled = true;
    GraphicsContext->SetVsync(true);
}

void GlfwWindow::DisableVSync()
{
    WindowData.bVsyncEnabled = false;
    GraphicsContext->SetVsync(false);
}

bool GlfwWindow::IsVSyncEnabled() const
{
    return WindowData.bVsyncEnabled;
}

void* GlfwWindow::GetWindowNativeHandle() const
{
    return Window;
}

IGraphicsContext* GlfwWindow::GetContext() const
{
    return GraphicsContext;
}

void GlfwWindow::Close()
{
    glfwSetWindowShouldClose(Window, GL_TRUE);
    WindowData.bGameRunning = false;
}

void GlfwWindow::SetMouseVisible(bool bMouseVisible)
{
    if (bMouseVisible)
    {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void GlfwWindow::BindWindowCallbacks()
{
    glfwSetWindowUserPointer(Window, &WindowData);

    glfwSetCursorPosCallback(Window, [](GLFWwindow* window, double xpos, double ypos) {
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

    glfwSetKeyCallback(Window, [](GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::kKeyPressed : EventType::kKeyReleased;
            event.Key = {(KeyCode)key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER)};

            windowData->Callback(event);
        }
    });

    glfwSetMouseButtonCallback(Window, [](GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS) ? EventType::kMouseButtonPressed : EventType::kMouseButtonReleased;
            event.MouseButtonState = {(MouseButton)button, windowData->MousePosition};
            windowData->Callback(event);
        }
    });

    glfwSetWindowFocusCallback(Window, [](GLFWwindow* window, int32_t focused) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = (focused == GL_TRUE) ? EventType::kGainedFocus : EventType::kLostFocus;
            windowData->Callback(event);
        }
    });

    glfwSetScrollCallback(Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (windowData->Callback)
        {
            Event event{};
            event.Type = EventType::kMouseWheelScrolled;
            event.MouseWheel.Delta = {xoffset, yoffset};
            windowData->Callback(event);
        }
    });

    glfwSetWindowCloseCallback(Window, [](GLFWwindow* window) {
        GlfwWindowData* windowData = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        windowData->bGameRunning = false;
    });
}
