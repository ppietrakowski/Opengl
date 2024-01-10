#pragma once

#include "Imgui/imgui.h"
#include "vertex_array.h"
#include "shader.h"
#include "game_layer.h"
#include "graphics_context.h"
#include "window.h"

#include <cstdint>
#include <string>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>

class Game
{
public:
    Game(const WindowSettings& settings);
    ~Game();

public:

    void Run();
    bool IsRunning() const;
    void Quit();

    void SetMouseVisible(bool bMouseVisible);

    void AddLayer(std::unique_ptr<ILayer>&& gameLayer);
    void RemoveLayer(std::type_index index);

private:
    std::unique_ptr<Window> m_Window;
    GraphicsContext* m_GraphicsContext;
    ImGuiContext* m_ImguiContext;
    std::vector<std::unique_ptr<ILayer>> m_Layers;

private:
    bool InitializeImGui();
    void RunImguiFrame();

    void BindWindowEvents();
};

inline std::chrono::nanoseconds GetNow()
{
    using std::chrono::system_clock;
    return system_clock::now().time_since_epoch();
}