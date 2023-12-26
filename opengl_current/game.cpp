#include "game.h"

#include "error_macros.h"
#include "renderer.h"
#include "logging.h"

#include <glm/gtc/matrix_transform.hpp>

static Game* game_instance_ = nullptr;

Game::Game(const WindowSettings& settings) :
    imgui_context_{ nullptr } {
    Logging::Initialize();
    window_ = Window::Create(settings);
    graphics_context_ = window_->GetContext();
    
    // initialize subsystems
    Renderer::Initialize();
    Renderer::UpdateProjection(static_cast<float>(settings.width), static_cast<float>(settings.height), 45.0f, 0.01f);

    BindWindowEvents();
    InitializeImGui();
    game_instance_ = this;
}

Game::~Game() {
    layers_.clear();

    // deinitialize all libraries
    Renderer::Quit();

    graphics_context_->DeinitializeImGui();
    ImGui::DestroyContext();

    Logging::Quit();
    game_instance_ = nullptr;
}

void Game::Run() {
    time_milliseconds_t delta_seconds = time_milliseconds_t::zero();
    auto last_frame_time = GetNow();
    bool first_frame = true;

    while (window_->IsOpen()) {
        for (const std::unique_ptr<Layer>& layer : layers_) {
            layer->OnUpdate(delta_seconds);
        }

        RenderCommand::Clear();

        // calculate delta time using chrono library
        auto now = GetNow();

        if (first_frame) {
            delta_seconds = (now - last_frame_time);
            first_frame = false;
        } else {
            // average delta seconds to keep more meaningfull frame time
            delta_seconds = ((now - last_frame_time) + delta_seconds) / 2;
        }

        last_frame_time = now;

        // broadcast render command
        for (const std::unique_ptr<Layer>& layer : layers_) {
            layer->OnRender(delta_seconds);
        }

        RunImguiFrame();
        window_->Update();
    }
}

bool Game::IsRunning() const {
    return window_->IsOpen();
}

void Game::Quit() {
    window_->Close();
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
    graphics_context_->InitializeForImGui();
    return true;
}

void Game::RunImguiFrame() {
    graphics_context_->ImGuiBeginFrame();
    ImGui::NewFrame();

    // broadcast imgui frame draw
    for (const std::unique_ptr<Layer>& layer : layers_) {
        layer->OnImguiFrame();
    }

    ImGui::Render();
    graphics_context_->ImGuiDrawFrame();
    ImGui::EndFrame();
    graphics_context_->UpdateImGuiViewport();
}

void Game::BindWindowEvents() {
    window_->SetEventCallback([this](const Event& evt) {
        // events in layer are processed from last to first
        for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
            std::unique_ptr<Layer>& layer = *it;

            if (layer->OnEvent(evt)) {
                return;
            }
        }
    });
}

void Game::SetMouseVisible(bool mouse_visible) {
    window_->SetMouseVisible(mouse_visible);
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