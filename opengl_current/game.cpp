#include "game.h"

#include "error_macros.h"
#include "renderer.h"
#include "logging.h"

#include <glm/gtc/matrix_transform.hpp>

static Game* s_GameInstance = nullptr;

Game::Game(const WindowSettings& settings) :
    ImguiContext{nullptr}
{
    Logging::Initialize();
    Window = IWindow::Create(settings);
    GraphicsContext = Window->GetContext();

    // initialize subsystems
    Renderer::Initialize();
    Renderer::UpdateProjection(CameraProjection{settings.Width, settings.Height, 45.0f});

    BindWindowEvents();
    InitializeImGui();
    s_GameInstance = this;
}

Game::~Game()
{
    Layers.clear();

    // deinitialize all libraries
    Renderer::Quit();

    GraphicsContext->DeinitializeImGui();
    ImGui::DestroyContext();

    Logging::Quit();
    s_GameInstance = nullptr;
}

void Game::Run()
{
    std::chrono::nanoseconds deltaSeconds{std::chrono::nanoseconds::zero()};
    auto lastFrameTime = GetNow();

    while (Window->IsOpen())
    {
        for (const std::unique_ptr<ILayer>& layer : Layers)
        {
            layer->Update(deltaSeconds);
        }

        RenderCommand::Clear();

        // calculate delta time using chrono library
        auto now = GetNow();

        if (deltaSeconds == std::chrono::nanoseconds::zero())
        {
            deltaSeconds = (now - lastFrameTime);
        }
        else
        {
            // average delta seconds to keep more meaningfull frame time
            deltaSeconds = ((now - lastFrameTime) + deltaSeconds) / 2;
        }

        lastFrameTime = now;

        // broadcast render command
        for (const std::unique_ptr<ILayer>& layer : Layers)
        {
            layer->Render(deltaSeconds);
        }

        RunImguiFrame();
        Window->Update();
    }
}

bool Game::IsRunning() const
{
    return Window->IsOpen();
}

void Game::Quit()
{
    Window->Close();
}

bool Game::InitializeImGui()
{
    ImguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(ImguiContext);

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    GraphicsContext->InitializeForImGui();
    return true;
}

void Game::RunImguiFrame()
{
    GraphicsContext->ImGuiBeginFrame();
    ImGui::NewFrame();

    // broadcast imgui frame draw
    for (const std::unique_ptr<ILayer>& layer : Layers)
    {
        layer->OnImguiFrame();
    }

    ImGui::Render();
    GraphicsContext->ImGuiDrawFrame();
    ImGui::EndFrame();
    GraphicsContext->UpdateImGuiViewport();
}

void Game::BindWindowEvents()
{
    Window->SetEventCallback([this](const Event& evt) {
        // events in layer are processed from last to first
        for (auto it = Layers.rbegin(); it != Layers.rend(); ++it)
        {
            std::unique_ptr<ILayer>& layer = *it;

            if (layer->OnEvent(evt))
            {
                return;
            }
        }
    });
}

void Game::SetMouseVisible(bool bMouseVisible)
{
    Window->SetMouseVisible(bMouseVisible);
}

void Game::AddLayer(std::unique_ptr<ILayer>&& gameLayer)
{
    Layers.emplace_back(std::move(gameLayer));
}

void Game::RemoveLayer(std::type_index index)
{
    auto it = std::remove_if(Layers.begin(),
        Layers.end(),
        [index](const std::unique_ptr<ILayer>& layer) { return layer->GetTypeIndex() == index; });

    if (it != Layers.end())
    {
        Layers.erase(it);
    }
}