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
    unsigned int Width;
    unsigned int Height;
    std::string Title;
};

typedef std::chrono::duration<float, std::ratio<1, 1>> TimeSeconds;

struct WindowData
{
    std::function<void(const Event&)> EventCallback;
    bool MouseVisible{ true };
    glm::ivec2 WindowPosition;
    glm::ivec2 WindowSize;
    glm::vec2 MousePosition;
    glm::vec2 LastMousePosition;
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
    virtual bool IsKeyDown(int key) const override;

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

