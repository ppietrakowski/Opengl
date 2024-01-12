#include "game.h"

#include "error_macros.h"
#include "renderer.h"
#include "logging.h"

#include <glm/gtc/matrix_transform.hpp>

Game::Game(const WindowSettings& settings) :
    m_ImguiContext{nullptr}
{
    Logging::Initialize();
    m_Window = std::make_unique<Window>(settings);
    m_GraphicsContext = m_Window->GetContext();

    // initialize subsystems
    Renderer::Initialize();
    Renderer::UpdateProjection(CameraProjection{settings.Width, settings.Height, 45.0f});

    BindWindowEvents();
    InitializeImGui();
}

Game::~Game()
{
    m_Layers.clear();

    // deinitialize all libraries
    Renderer::Quit();

    m_GraphicsContext->DeinitializeImGui();
    ImGui::DestroyContext();

    Logging::Quit();
}

void Game::Run()
{
    std::chrono::nanoseconds deltaSeconds{std::chrono::nanoseconds::zero()};
    auto lastFrameTime = GetNow();

    while (m_Window->IsOpen())
    {
        for (const std::unique_ptr<ILayer>& layer : m_Layers)
        {
            layer->Update(deltaSeconds);
        }

        RenderCommand::Clear();

        // calculate delta time using chrono library
        auto now = GetNow();

        if (deltaSeconds == std::chrono::nanoseconds::zero())
        {
            deltaSeconds = (now - lastFrameTime);
        } else
        {
            // average delta seconds to keep more meaningfull frame time
            deltaSeconds = ((now - lastFrameTime) + deltaSeconds) / 2;
        }

        lastFrameTime = now;

        // broadcast render command
        for (const std::unique_ptr<ILayer>& layer : m_Layers)
        {
            layer->Render(deltaSeconds);
        }

        RunImguiFrame();
        m_Window->Update();
    }
}

bool Game::IsRunning() const
{
    return m_Window->IsOpen();
}

void Game::Quit()
{
    m_Window->Close();
}

bool Game::InitializeImGui()
{
    m_ImguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_ImguiContext);

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    m_GraphicsContext->InitializeForImGui();
    return true;
}

void Game::RunImguiFrame()
{
    m_GraphicsContext->ImGuiBeginFrame();
    ImGui::NewFrame();

    // broadcast imgui frame draw
    for (const std::unique_ptr<ILayer>& layer : m_Layers)
    {
        layer->OnImguiFrame();
    }

    ImGui::Render();
    m_GraphicsContext->ImGuiDrawFrame();
    ImGui::EndFrame();
    m_GraphicsContext->UpdateImGuiViewport();
}

void Game::BindWindowEvents()
{
    m_Window->SetEventCallback([this](const Event& evt) {
        // events in layer are processed from last to first
        for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
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
    m_Window->SetMouseVisible(bMouseVisible);
}

void Game::AddLayer(std::unique_ptr<ILayer>&& gameLayer)
{
    m_Layers.emplace_back(std::move(gameLayer));
}

void Game::RemoveLayer(std::type_index index)
{
    auto it = std::remove_if(m_Layers.begin(),
        m_Layers.end(),
        [index](const std::unique_ptr<ILayer>& layer) { return layer->GetTypeIndex() == index; });

    if (it != m_Layers.end())
    {
        m_Layers.erase(it);
    }
}