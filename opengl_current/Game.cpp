#include "Game.hpp"

#include "ErrorMacros.hpp"
#include "Renderer.hpp"
#include "Logging.hpp"

#include "Renderer2d.hpp"
#include "Debug.hpp"
#include "DeltaClock.hpp"
#include "ResourceManager.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

#include <GLFW/glfw3.h>

using ChronoDeltaTimeClock = std::chrono::steady_clock;
constexpr ChronoDeltaTimeClock::duration MaxMillisecondsDeltaTime = std::chrono::duration_cast<ChronoDeltaTimeClock::duration>(milliseconds_float_t(1000.0f));
constexpr ChronoDeltaTimeClock::duration MinMillisecondsDeltaTime = std::chrono::duration_cast<ChronoDeltaTimeClock::duration>(milliseconds_float_t(16.66667));


struct GlfwLib
{
    GlfwLib()
    {
        glfwSetErrorCallback([](int code, const char* description)
        {
            ENG_LOG_ERROR("Glfw error {} : {}", code, description);
        });

        CRASH_EXPECTED_TRUE_MSG(glfwInit(), "Glfw initialization failed");

#if defined(DEBUG) || defined(_DEBUG)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    ~GlfwLib()
    {
        glfwTerminate();
    }
};

Game::Game(const WindowSettings& settings) :
    m_LoggingInitializer{},
    m_GlfwLib(std::make_unique<GlfwLib>()),
    m_Window(settings),
    m_ImguiContext{nullptr}
{
    // initialize subsystems
    Renderer::Initialize();
    Renderer2D::Initialize();
    Renderer2D::UpdateProjection(CameraProjection{settings.Width, settings.Height, 45.0f});
    Renderer::UpdateProjection(CameraProjection{settings.Width, settings.Height, 45.0f});

    InitializeImGui();

    m_LevelContext.CreateNewEmpty();

    Debug::InitializeDebugDraw(ResourceManager::GetShader("assets/shaders/unshaded.shd"));
    Renderer2D::SetDrawShader(ResourceManager::GetShader("assets/shaders/sprite_2d.shd"));
    ImGuizmo::SetOrthographic(false);
}

std::shared_ptr<Game> Game::CreateGame(const WindowSettings& settings)
{
    std::shared_ptr<Game> game(new Game(settings));
    s_GameInstance = game;
    game->BindWindowEvents();

    return game;
}

Game::~Game()
{
    // deinitialize all libraries
    Renderer2D::Quit();
    Debug::Quit();
    Renderer::Quit();

    m_Window.DeinitializeImGui();
    ImGui::DestroyContext();
}

using DeltaTimeClock = DeltaClockBase<ChronoDeltaTimeClock>;

void Game::Run()
{
    DeltaTimeClock clock{};
    auto frameTime = clock.GetDelta();

    while (m_Window.IsOpen())
    {
        // calculate delta time using chrono library
        frameTime = (frameTime + clock.GetDelta()) / 2;
        clock.PerformTick();

        while (frameTime.count() > 0)
        {
            auto deltaTime = std::clamp(frameTime, MinMillisecondsDeltaTime, MaxMillisecondsDeltaTime);

            for (const std::unique_ptr<IGameLayer>& layer : m_Layers)
            {
                layer->Update(Duration{deltaTime});
            }

            m_LevelContext.CurrentLevel->BroadcastUpdate(deltaTime);

            frameTime -= deltaTime;
        }
        

        RenderCommand::Clear();
        std::shared_ptr<Level> level = m_LevelContext.CurrentLevel;
        Renderer::BeginScene(level->CameraPosition, level->CameraRotation, level->GetLightsData());
        Level::BeginScene(Renderer::GetProjectionMatrix(), Renderer::GetViewMatrix(), Renderer::GetViewport());
        Debug::BeginScene(Renderer::GetProjectionViewMatrix());

        m_LevelContext.CurrentLevel->BroadcastRender();

        // broadcast render command
        for (const std::unique_ptr<IGameLayer>& layer : m_Layers)
        {
            layer->Render();
        }

        Debug::FlushDrawDebug();
        Renderer2D::FlushDraw();

        RunImguiFrame();
        Renderer::EndScene();

        m_Window.Update();
    }
}

bool Game::IsRunning() const
{
    return m_Window.IsOpen();
}

void Game::Quit()
{
    m_Window.Close();
}

bool Game::OnKeyDown(KeyCode::Index keyCode)
{
    for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
    {
        auto& layer = *it;

        if (layer->OnKeyDown(keyCode))
        {
            break;
        }
    }

    return false;
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
    m_Window.InitializeImGui();
    return true;
}

void Game::RunImguiFrame()
{
    m_Window.ImGuiBeginFrame();
    ImGuizmo::SetOrthographic(false);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(m_Window.GetWidth()), static_cast<float>(m_Window.GetHeight()));

    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    // broadcast imgui frame draw
    for (const std::unique_ptr<IGameLayer>& layer : m_Layers)
    {
        layer->OnImguiFrame();
    }

    for (const std::unique_ptr<IGameLayer>& layer : m_Layers)
    {
        layer->OnImgizmoFrame();
    }

    ImGui::Render();
    m_Window.ImGuiDrawFrame();
    ImGui::EndFrame();
    m_Window.ImGuiUpdateViewport();
}

void Game::BindWindowEvents()
{
    m_Window.SetWindowMessageHandler(s_GameInstance.lock());

#if 0
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

        if (evt.Type == EventType::WindowResized)
        {
            CameraProjection projection(evt.Size.Width, evt.Size.Height);
            Renderer2D::UpdateProjection(projection);
            Renderer::UpdateProjection(projection);
        }
    });
#endif
}

void Game::SetMouseVisible(bool bMouseVisible)
{
    m_Window.SetMouseVisible(bMouseVisible);
}

void Game::AddLayer(std::unique_ptr<IGameLayer> gameLayer)
{
    m_TypeIndexToLayerIndex.try_emplace(gameLayer->GetTypeIndex(), m_Layers.size());
    m_Layers.emplace_back(std::move(gameLayer));
}

void Game::RemoveLayer(std::type_index index)
{
    const size_t* i = FindMap(m_TypeIndexToLayerIndex, index);

    if (i)
    {
        m_Layers.erase(m_Layers.begin() + *i);
        m_TypeIndexToLayerIndex.clear();

        size_t idx = 0;
        for (auto& layer : m_Layers)
        {
            m_TypeIndexToLayerIndex[layer->GetTypeIndex()] = idx++;
        }
    }
}

void LevelContext::CreateNewEmpty()
{
    CurrentLevel = std::make_shared<Level>();
}