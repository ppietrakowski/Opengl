#include "opengl_graphics_context.h"
#include "glfw_window.h"
#include "error_macros.h"
#include "logging.h"


GlfwWindow::GlfwWindow(const WindowSettings& settings) {
    glfwSetErrorCallback([](int code, const char* description) {
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
    glm::dvec2 mouse_position;
    glfwGetCursorPos(window_, &mouse_position.x, &mouse_position.y);
    WindowData& game_window_data = window_data_;
    game_window_data.mouse_position = mouse_position;
    game_window_data.last_mouse_position = game_window_data.mouse_position;

    glfwGetWindowPos(window_, &game_window_data.window_position.x, &game_window_data.window_position.y);
    glfwGetWindowSize(window_, &game_window_data.window_size.x, &game_window_data.window_size.y);

    BindWindowCallbacks();
    context_ = new OpenGlGraphicsContext(window_);
    input_ = new GlfwInput(window_);
}

GlfwWindow::~GlfwWindow() {
    delete input_;
    delete context_;
    glfwDestroyWindow(window_);
}

void GlfwWindow::Update() {
    glfwPollEvents();
    context_->SwapBuffers();
    input_->Update(window_data_);
}

uint32_t GlfwWindow::GetWidth() const {
    return window_data_.window_size.x;
}

uint32_t GlfwWindow::GetHeight() const {
    return window_data_.window_size.y;
}

glm::ivec2 GlfwWindow::GetWindowPosition() const {
    return window_data_.window_position;
}

glm::vec2 GlfwWindow::GetMousePosition() const {
    return window_data_.mouse_position;
}

glm::vec2 GlfwWindow::GetLastMousePosition() const {
    return window_data_.last_mouse_position;
}

bool GlfwWindow::IsOpen() const {
    return window_data_.game_running;
}

void GlfwWindow::SetEventCallback(const EventCallback& callback) {
    window_data_.event_callback = callback;
}

void GlfwWindow::EnableVSync() {
    window_data_.vsync_enabled = true;
    context_->SetVsync(true);
}

void GlfwWindow::DisableVSync() {
    window_data_.vsync_enabled = false;
    context_->SetVsync(false);
}

bool GlfwWindow::IsVSyncEnabled() const {
    return window_data_.vsync_enabled;
}

void* GlfwWindow::GetWindowNativeHandle() const {
    return window_;
}

GraphicsContext* GlfwWindow::GetContext() const {
    return context_;
}

void GlfwWindow::Close() {
    glfwSetWindowShouldClose(window_, GL_TRUE);
    window_data_.game_running = false;
}

void GlfwWindow::SetMouseVisible(bool mouse_visible) {
    if (mouse_visible) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void GlfwWindow::BindWindowCallbacks() {
    glfwSetWindowUserPointer(window_, &window_data_);

    glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
        game_window_data->last_mouse_position = game_window_data->mouse_position;
        game_window_data->mouse_position = glm::vec2{ xpos, ypos };

        if (game_window_data->event_callback) {
            Event evt{};
            evt.type = EventType::kMouseMoved;
            evt.mouse_move.mouse_position = game_window_data->mouse_position;
            evt.mouse_move.last_mouse_position = game_window_data->last_mouse_position;
            game_window_data->event_callback(evt);
        }
    });

    glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (game_window_data->event_callback) {
            Event event{};
            event.type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::kKeyPressed : EventType::kKeyReleased;
            event.key = { (KeyCode)key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER) };

            game_window_data->event_callback(event);
        }
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (game_window_data->event_callback) {
            Event event{};
            event.type = (action == GLFW_PRESS) ? EventType::kMouseButtonPressed : EventType::kMouseButtonReleased;
            event.mouse_button = { (MouseCode)button, game_window_data->mouse_position };
            game_window_data->event_callback(event);
        }
    });

    glfwSetWindowFocusCallback(window_, [](GLFWwindow* window, int focused) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (game_window_data->event_callback) {
            Event event{};
            event.type = (focused == GL_TRUE) ? EventType::kGainedFocus : EventType::kLostFocus;
            game_window_data->event_callback(event);
        }
    });

    glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (game_window_data->event_callback) {
            Event event{};
            event.type = EventType::kMouseWheelScrolled;
            event.mouse_wheel.delta = { xoffset, yoffset };
            game_window_data->event_callback(event);
        }
    });

    glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
        game_window_data->game_running = false;
    });
}
