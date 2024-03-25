#pragma once

#include "Imgui/imgui.h"
#include "VertexArray.hpp"
#include "Shader.hpp"
#include "GameLayer.hpp"
#include "GraphicsContext.hpp"
#include "Window.hpp"

#include "Level.hpp"
#include "Logging.hpp"
#include "imgizmo/ImGuizmo.h"

#include <cstdint>
#include <string>
#include <unordered_map>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>

struct LevelContext
{
    std::shared_ptr<Level> CurrentLevel;
    void CreateNewEmpty();
};

struct LoggingInitializer
{
    LoggingInitializer()
    {
        Logging::Initialize();
    }

    ~LoggingInitializer()
    {
        Logging::Quit();
    }
};

struct GlfwLib;

class Game : public IWindowMessageHandler
{

public:
    static std::shared_ptr<Game> CreateGame(const WindowSettings& settings);
    ~Game();

public:

    void Run();
    bool IsRunning() const;
    void Quit();

    void SetMouseVisible(bool bMouseVisible);

    void AddLayer(std::unique_ptr<IGameLayer> gameLayer);
    void RemoveLayer(std::type_index index);

    bool IsMouseVisible() const
    {
        return m_Window.IsMouseVisible();
    }

    std::shared_ptr<Level> GetCurrentLevel() const
    {
        return m_LevelContext.CurrentLevel;
    }

    virtual bool OnKeyDown(KeyCode::Index keyCode) override;

private:
    LoggingInitializer m_LoggingInitializer;
    std::unique_ptr<GlfwLib> m_GlfwLib;
    Window m_Window;
    ImGuiContext* m_ImguiContext;
    std::vector<std::unique_ptr<IGameLayer>> m_Layers;
    std::unordered_map<std::type_index, size_t> m_TypeIndexToLayerIndex;
    LevelContext m_LevelContext;

    static inline std::weak_ptr<Game> s_GameInstance;

private:
    Game(const WindowSettings& settings);

private:
    bool InitializeImGui();
    void RunImguiFrame();

    void BindWindowEvents();
};