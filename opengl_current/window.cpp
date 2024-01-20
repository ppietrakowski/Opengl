#include "window.h"

#include "error_macros.h"
#include "input.h"
#include "logging.h"

#include <GLFW/glfw3.h>

Window::Window(const WindowSettings& settings) {
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
    glm::dvec2 mousePosition;
    glfwGetCursorPos(window_, &mousePosition.x, &mousePosition.y);
    window_data_.mouse_position = mousePosition;
    window_data_.last_mouse_position = window_data_.mouse_position;

    glfwGetWindowPos(window_, &window_data_.window_position.x, &window_data_.window_position.y);

    glm::ivec2 window_size{0, 0};
    glfwGetWindowSize(window_, &window_size.x, &window_size.y);
    window_data_.window_size = window_size;

    BindWindowCallbacks();
    graphics_context_ = new GraphicsContext(window_);
    input_ = new Input(window_);
}

Window::~Window() {
    delete input_;
    delete graphics_context_;
    glfwDestroyWindow(window_);
}

void Window::Update() {
    glfwPollEvents();
    graphics_context_->SwapBuffers();
    input_->Update(window_data_);
}

uint32_t Window::GetWidth() const {
    return window_data_.window_size.x;
}

uint32_t Window::GetHeight() const {
    return window_data_.window_size.y;
}

glm::ivec2 Window::GetWindowPosition() const {
    return window_data_.window_position;
}

glm::vec2 Window::GetMousePosition() const {
    return window_data_.mouse_position;
}

glm::vec2 Window::GetLastMousePosition() const {
    return window_data_.last_mouse_position;
}

bool Window::IsOpen() const {
    return window_data_.game_running;
}

void Window::SetEventCallback(const EventCallback& callback) {
    window_data_.event_callback = callback;
}

void Window::EnableVSync() {
    window_data_.vsync_enabled = true;
    graphics_context_->SetVsync(true);
}

void Window::DisableVSync() {
    window_data_.vsync_enabled = false;
    graphics_context_->SetVsync(false);
}

bool Window::IsVSyncEnabled() const {
    return window_data_.vsync_enabled;
}

void* Window::GetWindowNativeHandle() const {
    return window_;
}

GraphicsContext* Window::GetContext() const {
    return graphics_context_;
}

void Window::Close() {
    glfwSetWindowShouldClose(window_, GL_TRUE);
    window_data_.game_running = false;
}

void Window::SetMouseVisible(bool mouse_visible) {
    if (mouse_visible) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Window::BindWindowCallbacks() {
    glfwSetWindowUserPointer(window_, &window_data_);

    glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));
        window_data->last_mouse_position = window_data->mouse_position;
        window_data->mouse_position = glm::vec2{xpos, ypos};

        if (window_data->event_callback) {
            Event evt{};
            evt.type = EventType::kMouseMoved;
            evt.mouse_move.mouse_position = window_data->mouse_position;
            evt.mouse_move.last_mouse_position = window_data->last_mouse_position;
            window_data->event_callback(evt);
        }
    });

    glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback) {
            Event event{};
            event.type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::kKeyPressed : EventType::kKeyReleased;
            event.key = {(KeyCode)key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER)};

            window_data->event_callback(event);
        }
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback) {
            Event event{};
            event.type = (action == GLFW_PRESS) ? EventType::kMouseButtonPressed : EventType::kMouseButtonReleased;
            event.mouse_button = {(MouseButton)button, window_data->mouse_position};
            window_data->event_callback(event);
        }
    });

    glfwSetWindowFocusCallback(window_, [](GLFWwindow* window, int focused) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback) {
            Event event{};
            event.type = (focused == GL_TRUE) ? EventType::kGainedFocus : EventType::kLostFocus;
            window_data->event_callback(event);
        }
    });

    glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset) {
        GlfwWindowData* window_data = reinterpret_cast<GlfwWindowData*>(glfwGetWindowUserPointer(window));

        if (window_data->event_callback) {
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
