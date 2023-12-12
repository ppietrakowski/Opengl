#include "Game.h"

#include "ErrorMacros.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_impl_glfw.h"

#include "Renderer.h"
#include "Logging.h"

#include <functional>
#include <glm/gtc/matrix_transform.hpp>

static Game* GameInstance = NULL;

static std::chrono::milliseconds GetNowTimestamp()
{
    using namespace std::chrono;
    auto duration = steady_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(duration);
}

Game::Game(const WindowSettings& settings) :
    _imguiContext{ nullptr }
{
    Logging::Initialize();

    CRASH_EXPECTED_TRUE(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    _window = glfwCreateWindow(settings.Width, settings.Height, settings.Title.c_str(), NULL, NULL);
    CRASH_EXPECTED_TRUE(_window != NULL);
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);
    GLenum errorCode = glewInit();
    CRASH_EXPECTED_TRUE_MSG(errorCode == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(errorCode)));

    Renderer::Initialize();
    Renderer::UpdateProjection(static_cast<float>(settings.Width), static_cast<float>(settings.Height), 45.0f, 0.01f);

    glm::dvec2 pos;
    glfwGetCursorPos(_window, &pos.x, &pos.y);
    WindowData& GameWindowData = _windowData;
    GameWindowData.MousePosition = pos;

    glfwGetWindowPos(_window, &GameWindowData.WindowPosition.x, &GameWindowData.WindowPosition.y);
    glfwGetWindowSize(_window, &GameWindowData.WindowSize.x, &GameWindowData.WindowSize.y);

    double lastMouseX;
    double lastMouseY;

    glfwGetCursorPos(_window, &lastMouseX, &lastMouseY);
    
    _windowData.MousePosition = glm::vec2{ lastMouseX, lastMouseY };
    _windowData.LastMousePosition = glm::vec2{ lastMouseX, lastMouseY };
    BindWindowEvents();

    InitializeImGui();
    GameInstance = this;
}

Game::~Game()
{
    for (Layer* layer : _layers)
    {
        delete layer;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    Renderer::Quit();

    glfwDestroyWindow(_window);
    glfwTerminate();

    Logging::Quit();

    GameInstance = NULL;
}

constexpr unsigned int ClearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

void Game::Run()
{
    TimeSeconds deltaSeconds = TimeSeconds::zero();
    std::chrono::milliseconds lastFrameTime = GetNowTimestamp();

    while (!glfwWindowShouldClose(_window))
    {
        for (Layer* layer : _layers)
        {
            layer->OnUpdate(deltaSeconds.count());
        }

        RenderCommand::Clear(ClearFlags);

        std::chrono::milliseconds time = GetNowTimestamp();
        deltaSeconds = (time - lastFrameTime);
        lastFrameTime = time;

        for (Layer* layer : _layers)
        {
            layer->OnRender(deltaSeconds.count());
        }

        RunImguiFrame(deltaSeconds);

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}

bool Game::IsRunning() const
{
    return !glfwWindowShouldClose(_window);
}

void Game::Quit()
{
    glfwSetWindowShouldClose(_window, GL_TRUE);
}

bool Game::InitializeImGui()
{
    _imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(_imguiContext);

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    bool imguiInitialized = ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ERR_FAIL_EXPECTED_TRUE_V(imguiInitialized, false);

    const char* glslVersion = "#version 430 core";
    imguiInitialized = ImGui_ImplOpenGL3_Init(glslVersion);
    return imguiInitialized;
}

void Game::RunImguiFrame(TimeSeconds deltaSeconds)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // imgui draw
    for (Layer* layer : _layers)
    {
        layer->OnImguiFrame();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupCurrentContext);
    }
}

void Game::BindWindowEvents()
{
    glfwSetWindowUserPointer(_window, &_windowData);

    glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xpos, double ypos)
    {
        WindowData* gameWindowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
        gameWindowData->LastMousePosition = gameWindowData->MousePosition;
        gameWindowData->MousePosition = glm::vec2{ xpos, ypos };

        if (gameWindowData->EventCallback)
        {
            Event evt{};
            evt.Type = EEventType::MouseMoved;
            evt.MouseMove.MousePosition = gameWindowData->MousePosition;
            evt.MouseMove.LastMousePosition = gameWindowData->LastMousePosition;
            gameWindowData->EventCallback(evt);
        }
    });

    glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        WindowData* gameWindowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (gameWindowData->EventCallback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EEventType::KeyPressed : EEventType::KeyReleased;
            event.Key = { key, scancode, (bool)(mods & GLFW_MOD_ALT), (bool)(mods & GLFW_MOD_CONTROL), (bool)(mods & GLFW_MOD_SHIFT), (bool)(mods & GLFW_MOD_SUPER) };

            gameWindowData->EventCallback(event);
        }
    });

    glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods)
    {
        WindowData* gameWindowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (gameWindowData->EventCallback)
        {
            Event event{};
            event.Type = (action == GLFW_PRESS) ? EEventType::MouseButtonPressed : EEventType::MouseButtonReleased;
            event.MouseButton = { button, gameWindowData->MousePosition };
            gameWindowData->EventCallback(event);
        }
    });

    glfwSetWindowFocusCallback(_window, [](GLFWwindow* window, int focused)
    {
        WindowData* gameWindowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (gameWindowData->EventCallback)
        {
            Event event{};
            event.Type = (focused == GL_TRUE) ? EEventType::GainedFocus : EEventType::LostFocus;
            gameWindowData->EventCallback(event);
        }
    });

    glfwSetScrollCallback(_window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        WindowData* gameWindowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

        if (gameWindowData->EventCallback)
        {
            Event event{};
            event.Type = EEventType::MouseWheelScrolled;
            event.MouseWheel.Delta = { xoffset, yoffset };
            gameWindowData->EventCallback(event);
        }
    });

    _windowData.EventCallback = [&](const Event& evt)
    {
        for (auto it = _layers.rbegin(); it != _layers.rend(); ++it)
        {
            Layer* layer = *it;

            if (layer->OnEvent(evt))
            {
                return;
            }
        }
    };
}

glm::vec2 Game::GetMousePosition() const
{
    return _windowData.MousePosition;
}

glm::vec2 Game::GetLastMousePosition() const
{
    return _windowData.LastMousePosition;
}

void Game::SetMouseVisible(bool mouseVisible)
{
    if (mouseVisible)
    {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

bool Game::IsKeyDown(int key) const
{
    return glfwGetKey(_window, key);
}

void Game::AddLayer(Layer* gameLayer)
{
    _layers.push_back(gameLayer);
}

void Game::RemoveLayer(std::type_index index)
{
    auto it = std::remove_if(_layers.begin(),
        _layers.end(),
        [index](const Layer* layer) { return layer->GetTypeIndex() == index; });

    if (it != _layers.end())
    {
        delete* it;
        _layers.erase(it);
    }
}