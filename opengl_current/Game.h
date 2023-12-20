#pragma once

#include <cstdint>
#include <string>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Imgui/imgui.h"

#include "VertexArray.h"
#include "Shader.h"
#include "Mesh.h"
#include "GameLayer.h"

#include <functional>

struct WindowSettings {
    std::uint32_t width;
    std::uint32_t height;
    std::string title;
};

struct WindowData {
    std::function<void(const Event&)> event_callback;
    bool mouse_visible : 1{ true };
    glm::ivec2 window_position{ 0, 0 };
    glm::ivec2 window_size{ 0, 0 };
    glm::vec2 mouse_position{ 0, 0 };
    glm::vec2 last_mouse_position{ 0, 0 };
    bool game_running : 1{ true };
};

class Game : public IPlatform {
public:
    Game(const WindowSettings& settings);
    ~Game();

public:

    void Run();
    bool IsRunning() const;
    void Quit();

    glm::vec2 GetMousePosition() const override;
    glm::vec2 GetLastMousePosition() const override;
    void SetMouseVisible(bool mouse_visible) override;
    bool IsKeyDown(std::int32_t key) const override;

    void AddLayer(std::unique_ptr<Layer>&& game_layer);
    void RemoveLayer(std::type_index index);

private:
    GLFWwindow* window_;
    ImGuiContext* imgui_context_;
    WindowData window_data_;
    std::vector<std::unique_ptr<Layer>> layers_;

private:
    bool InitializeImGui();
    void RunImguiFrame();

    void BindWindowEvents();
};

