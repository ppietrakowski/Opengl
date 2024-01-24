#pragma once

#include "Imgui/imgui.h"
#include "vertex_array.h"
#include "shader.h"
#include "game_layer.h"
#include "graphics_context.h"
#include "window.h"

#include "level.h"
#include "imgizmo/ImGuizmo.h"

#include <cstdint>
#include <string>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>

struct LevelContext
{
    std::shared_ptr<Level> CurrentLevel;
    void CreateNewEmpty();
};

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

    void AddLayer(std::unique_ptr<Layer>&& gameLayer);
    void RemoveLayer(std::type_index index);

    bool IsMouseVisible() const
    {
        return m_Window->IsMouseVisible();
    }

    std::shared_ptr<Level> GetCurrentLevel() const
    {
        return m_LevelContext.CurrentLevel;
    }

private:
    std::unique_ptr<Window> m_Window;
    ImGuiContext* m_ImguiContext;
    std::vector<std::unique_ptr<Layer>> m_Layers;
    LevelContext m_LevelContext;

private:
    bool InitializeImGui();
    void RunImguiFrame();

    void BindWindowEvents();
};