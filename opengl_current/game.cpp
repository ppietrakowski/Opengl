#include "game.h"

#include "error_macros.h"
#include "renderer.h"
#include "logging.h"

#include "renderer_2d.h"

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

using ChronoDeltaTimeClock = std::chrono::steady_clock;
constexpr ChronoDeltaTimeClock::duration MaxMillisecondsDeltaTime = std::chrono::duration_cast<ChronoDeltaTimeClock::duration>(milliseconds_float_t(1000.0f));
constexpr ChronoDeltaTimeClock::duration MinMillisecondsDeltaTime = std::chrono::duration_cast<ChronoDeltaTimeClock::duration>(milliseconds_float_t(16.66667));

Game::Game(const WindowSettings& settings) :
    m_ImguiContext{nullptr}
{
    Logging::Initialize();
    m_Window = std::make_unique<Window>(settings);

    // initialize subsystems
    Renderer::Initialize();
    Renderer::UpdateProjection(CameraProjection{settings.Width, settings.Height, 45.0f});

    BindWindowEvents();
    InitializeImGui();

    ImGuizmo::SetOrthographic(false);
}

Game::~Game()
{
    // deinitialize all libraries
    Renderer::Quit();

    m_Window->DeinitializeImGui();
    ImGui::DestroyContext();

    Logging::Quit();
}

template<typename chrono_clock, typename duration = chrono_clock::duration>
class delta_clock_base
{
public:
    using time_point_t = chrono_clock::time_point;
    using duration_t = duration;

    delta_clock_base()
    {
        PerformTick();
        PerformTick();
    }

    void PerformTick()
    {
        m_Then = m_Now;
        m_Now = chrono_clock::now();
        m_Delta = std::chrono::duration_cast<duration>(m_Now - m_Then);
    }

    time_point_t GetNow() const
    {
        return m_Now;
    }

    duration_t GetDelta() const
    {
        return m_Delta;
    }

private:
    time_point_t m_Now;
    time_point_t m_Then;
    duration_t m_Delta;
};

using DeltaTimeClock = delta_clock_base<ChronoDeltaTimeClock>;

void Game::Run()
{
    DeltaTimeClock clock{};
    auto frameTime = clock.GetDelta();

    while (m_Window->IsOpen())
    {
        // calculate delta time using chrono library
        frameTime = (frameTime + clock.GetDelta()) / 2;
        clock.PerformTick();

        while (frameTime.count() > 0)
        {
            auto deltaTime = std::clamp(frameTime, MinMillisecondsDeltaTime, MaxMillisecondsDeltaTime);

            for (const std::unique_ptr<Layer>& layer : m_Layers)
            {
                layer->Update(Duration{deltaTime});
            }

            frameTime -= deltaTime;
        }

        RenderCommand::Clear();

        // broadcast render command
        for (const std::unique_ptr<Layer>& layer : m_Layers)
        {
            layer->Render();
        }

        RunImguiFrame();
        Renderer2D::FlushDraw();
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
    m_Window->InitializeImGui();
    return true;
}

void Game::RunImguiFrame()
{
    m_Window->ImGuiBeginFrame();
    ImGuizmo::SetOrthographic(false);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));

    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    // broadcast imgui frame draw
    for (const std::unique_ptr<Layer>& layer : m_Layers)
    {
        layer->OnImguiFrame();
    }

    for (const std::unique_ptr<Layer>& layer : m_Layers)
    {
        layer->OnImgizmoFrame();
    }

    ImGui::Render();
    m_Window->ImGuiDrawFrame();
    ImGui::EndFrame();
    m_Window->ImGuiUpdateViewport();
}

void Game::BindWindowEvents()
{
    m_Window->SetEventCallback([this](const Event& evt)
    {
        // events in layer are processed from last to first
        for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
        {
            std::unique_ptr<Layer>& layer = *it;

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

void Game::AddLayer(std::unique_ptr<Layer>&& gameLayer)
{
    m_Layers.emplace_back(std::move(gameLayer));
}

void Game::RemoveLayer(std::type_index index)
{
    auto it = std::remove_if(m_Layers.begin(),
        m_Layers.end(),
        [index](const std::unique_ptr<Layer>& layer)
    {
        return layer->GetTypeIndex() == index;
    });

    if (it != m_Layers.end())
    {
        m_Layers.erase(it);
    }
}