#include "opengl_graphics_context.h"
#include "glfw_window.h"
#include "error_macros.h"
#include "logging.h"


GlfwWindow::GlfwWindow(const WindowSettings& settings)
{
    glfwSetErrorCallback([](std::int32_t code, const char* description) {
        ELOG_ERROR(LOG_CORE, "Glfw error %i : %s", code, description);
    });

    // initialize glfw and create window with opengl 4.3 context
    CRASH_EXPECTED_TRUE(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    CRASH_EXPECTED_NOT_NULL(window_);

    // fill window data
    glm::dvec2 mouse_pos;
    glfwGetCursorPos(window_, &mouse_pos.x, &mouse_pos.y);
    window_data_.mouse_position = mouse_pos;
    window_data_.last_mouse_position = window_data_.mouse_position;

    glfwGetWindowPos(window_, &window_data_.window_position.x, &window_data_.window_position.y);
    glfwGetWindowSize(window_, &window_data_.window_size.x, &window_data_.window_size.y);

    BindWindowCallbacks();
    graphics_context_ = new OpenGlGraphicsContext(window_);
    input_ = new GlfwInput(window_);
}

GlfwWindow::~GlfwWindow()
{
    delete input_;
    delete graphics_context_;
    glfwDestroyWindow(window_);
}

void GlfwWindow::Update()
{
    glfwPollEvents();
    graphics_context_->SwapBuffers();
    input_->Update(window_data_);
}

std::int32_t GlfwWindow::GetWidth() const
{
    return window_data_.window_size.x;
}

std::int32_t GlfwWindow::GetHeight() const
{
    return window_data_.window_size.y;
}

glm::ivec2 GlfwWindow::GetWindowPosition() const
{
    return window_data_.window_position;
}

glm::vec2 GlfwWindow::GetMousePosition() const
{
    return window_data_.mouse_position;
}

glm::vec2 GlfwWindow::GetLastMousePosition() const
{
    return window_data_.last_mouse_position;
}

bool GlfwWindow::IsOpen() const
{
    return window_data_.game_running;
}

void GlfwWindow::SetEventCallback(const EventCallback& callback)
{
    window_data_.event_callback = callback;
}

void GlfwWindow::EnableVSync()
{
    window_data_.vsync_enabled = true;
    graphics_context_->SetVsync(true);
}

void GlfwWindow::DisableVSync()
{
    window_data_.vsync_enabled = false;
    graphics_context_->SetVsync(false);
}

bool GlfwWindow::IsVSyncEnabled() const
{
    return window_data_.vsync_enabled;
}

void* GlfwWindow::GetWindowNativeHandle() const
{
    return window_;
}

GraphicsContext* GlfwWindow::GetContext() const
{
    return graphics_context_;
}

void GlfwWindow::Close()
{
    glfwSetWindowShouldClose(window_, GL_TRUE);
    window_data_.game_running = false;
}

void GlfwWindow::SetMouseVisible(bool bMouseVisible)
{
    if (bMouseVisible)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void GlfwWindow::BindWindowCallbacks()
{
    glfwSetWindowUserPointer(window_, &window_data_);

    glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        window_data->last_mouse_position = window_data->mouse_position;
        window_data->mouse_position = glm::vec2{xpos, ypos};

        if (window_data->event_callback)
        {
            Event evt{};
            evt.type = EventType::kMouseMoved;
            evt.mouse_move.mouse_position = window_data->mouse_position;
            evt.mouse_move.last_mouse_position = window_data->last_mouse_position;
            window_data->event_callback(evt);
        }
    });

    glfwSetKeyCallback(window_, [](GLFWwindow* window, std::int32_t key, std::int32_t scancode, std::int32_t action, std::int32_t mods) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback)
        {
            Event event{};
            event.type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::kKeyPressed : EventType::kKeyReleased;
            event.key_event = {(KeyCode)key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER)};

            window_data->event_callback(event);
        }
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, std::int32_t button, std::int32_t action, std::int32_t mods) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback)
        {
            Event event{};
            event.type = (action == GLFW_PRESS) ? EventType::kMouseButtonPressed : EventType::kMouseButtonReleased;
            event.mouse_button_state = {(MouseButton)button, window_data->mouse_position};
            window_data->event_callback(event);
        }
    });

    glfwSetWindowFocusCallback(window_, [](GLFWwindow* window, std::int32_t focused) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback)
        {
            Event event{};
            event.type = (focused == GL_TRUE) ? EventType::kGainedFocus : EventType::kLostFocus;
            window_data->event_callback(event);
        }
    });

    glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback)
        {
            Event event{};
            event.type = EventType::kMouseWheelScrolled;
            event.mouse_wheel.delta = {xoffset, yoffset};
            window_data->event_callback(event);
        }
    });

    glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        window_data->game_running = false;
    });
}
