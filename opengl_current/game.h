#pragma once

#include "Imgui/imgui.h"
#include "vertex_array.h"
#include "shader.h"
#include "game_layer.h"
#include "graphics_context.h"
#include "window.h"

#include "imgizmo/ImGuizmo.h"

#include <cstdint>
#include <string>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>

class Game {
public:
    Game(const WindowSettings& settings);
    ~Game();

public:

    void Run();
    bool IsRunning() const;
    void Quit();

    void SetMouseVisible(bool mouse_visible);

    void AddLayer(std::unique_ptr<Layer>&& game_layer);
    void RemoveLayer(std::type_index index);

    bool IsMouseVisible() const {
        return window_->IsMouseVisible();
    }

private:
    std::unique_ptr<Window> window_;
    GraphicsContext* graphics_context_;
    ImGuiContext* imgui_context_;
    std::vector<std::unique_ptr<Layer>> layers_;

private:
    bool InitializeImGui();
    void RunImguiFrame();

    void BindWindowEvents();
};

inline std::chrono::nanoseconds GetNow() {
    using std::chrono::system_clock;
    return system_clock::now().time_since_epoch();
}