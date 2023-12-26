#include "game.h"

#include "error_macros.h"
#include "renderer.h"
#include "logging.h"

#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_impl_glfw.h"

#include <glm/gtc/matrix_transform.hpp>

static Game* game_instance_ = nullptr;

Game::Game(const WindowSettings& settings) :
    imgui_context_{ nullptr } {
    Logging::Initialize();

    // initialize glfw and create window with opengl 4.3 context
    CRASH_EXPECTED_TRUE(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window_ = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    CRASH_EXPECTED_NOT_NULL(window_);

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    GLenum error_code = glewInit();
    CRASH_EXPECTED_TRUE_MSG(error_code == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(error_code)));

    // initialize subsystems
    Renderer::Initialize();
    Renderer::UpdateProjection(static_cast<float>(settings.width), static_cast<float>(settings.height), 45.0f, 0.01f);

    BindWindowEvents();
    InitializeImGui();

    // fill window data
    glm::dvec2 mouse_position;
    glfwGetCursorPos(window_, &mouse_position.x, &mouse_position.y);
    WindowData& game_window_data = window_data_;
    game_window_data.mouse_position = mouse_position;
    game_window_data.last_mouse_position = game_window_data.mouse_position;

    glfwGetWindowPos(window_, &game_window_data.window_position.x, &game_window_data.window_position.y);
    glfwGetWindowSize(window_, &game_window_data.window_size.x, &game_window_data.window_size.y);

    game_instance_ = this;
}

Game::~Game() {
    layers_.clear();

    // deinitialize all libraries
    Renderer::Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();

    Logging::Quit();
    game_instance_ = nullptr;
}

constexpr uint32_t kClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

void Game::Run() {
    time_milliseconds_t delta_seconds = time_milliseconds_t::zero();
    auto last_frame_time = GetNow();
    bool first_frame = true;

    while (window_data_.game_running) {
        for (const std::unique_ptr<Layer>& layer : layers_) {
            layer->OnUpdate(delta_seconds);
        }

        RenderCommand::Clear(kClearFlags);

        // calculate delta time using chrono library
        auto now = GetNow();

        if (first_frame) {
            delta_seconds = (now - last_frame_time);
            first_frame = false;
        } else {
            delta_seconds = ((now - last_frame_time) + delta_seconds) / 2;
        }

        last_frame_time = now;

        // broadcast render command
        for (const std::unique_ptr<Layer>& layer : layers_) {
            layer->OnRender(delta_seconds);
        }

        RunImguiFrame();

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

bool Game::IsRunning() const {
    return window_data_.game_running;
}

void Game::Quit() {
    glfwSetWindowShouldClose(window_, GL_TRUE);
    window_data_.game_running = false;
}

bool Game::InitializeImGui() {
    imgui_context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context_);

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    bool imgui_initialized = ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ERR_FAIL_EXPECTED_TRUE_V(imgui_initialized, false);

    const char* glsl_version = "#version 430 core";
    imgui_initialized = ImGui_ImplOpenGL3_Init(glsl_version);
    return imgui_initialized;
}

void Game::RunImguiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // broadcast imgui frame draw
    for (const std::unique_ptr<Layer>& layer : layers_) {
        layer->OnImguiFrame();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();

    ImGuiIO& io = ImGui::GetIO();

    // update viewport if enabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void Game::BindWindowEvents() {
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
            event.key = { key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER) };

            game_window_data->event_callback(event);
        }
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData* game_window_data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (game_window_data->event_callback) {
            Event event{};
            event.type = (action == GLFW_PRESS) ? EventType::kMouseButtonPressed : EventType::kMouseButtonReleased;
            event.mouse_button = { button, game_window_data->mouse_position };
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

    window_data_.event_callback = [this](const Event& evt) {
        // events in layer are processed from last to first
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            std::unique_ptr<Layer>& layer = *it;

            if (layer->OnEvent(evt)) {
                return;
            }
        }
    };
}

glm::vec2 Game::GetMousePosition() const {
    return window_data_.mouse_position;
}

glm::vec2 Game::GetLastMousePosition() const {
    return window_data_.last_mouse_position;
}

void Game::SetMouseVisible(bool mouse_visible) {
    if (mouse_visible) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

bool Game::IsKeyDown(int32_t key) const {
    return glfwGetKey(window_, key);
}

void Game::AddLayer(std::unique_ptr<Layer>&& game_layer) {
    layers_.emplace_back(std::move(game_layer));
}

void Game::RemoveLayer(std::type_index index) {
    auto it = std::remove_if(layers_.begin(),
        layers_.end(),
        [index](const std::unique_ptr<Layer>& layer) { return layer->GetTypeIndex() == index; });

    if (it != layers_.end()) {
        layers_.erase(it);
    }
}