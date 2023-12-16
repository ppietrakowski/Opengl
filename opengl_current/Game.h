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

struct WindowSettings
{
    std::uint32_t Width;
    std::uint32_t Height;
    std::string Title;
};

typedef std::chrono::duration<float, std::ratio<1, 1>> TimeSeconds;

struct WindowData
{
    std::function<void(const Event&)> EventCallback;
    bool MouseVisible : 1{ true };
    glm::ivec2 WindowPosition{ 0, 0 };
    glm::ivec2 WindowSize{ 0, 0 };
    glm::vec2 MousePosition{ 0, 0 };
    glm::vec2 LastMousePosition{ 0, 0 };
    bool GameRunning : 1{ true };
};

class Game : public IPlatform
{
public:
    Game(const WindowSettings& settings);
    ~Game();

public:

    void Run();
    bool IsRunning() const;
    void Quit();

    virtual glm::vec2 GetMousePosition() const override;
    virtual glm::vec2 GetLastMousePosition() const override;
    virtual void SetMouseVisible(bool mouseVisible) override;
    virtual bool IsKeyDown(std::int32_t key) const override;

    void AddLayer(Layer* gameLayer);
    void RemoveLayer(std::type_index index);

private:
    GLFWwindow* _window;
    ImGuiContext* _imguiContext;
    WindowData _windowData;
    std::vector<Layer*> _layers;

private:
    bool InitializeImGui();
    void RunImguiFrame(TimeSeconds deltaSeconds);

    void BindWindowEvents();
};

