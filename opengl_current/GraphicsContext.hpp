#pragma once

class Window;

class GraphicsContext
{
    friend class Window;

public:
    GraphicsContext() = default;
    ~GraphicsContext() = default;

public:
    void InitializeForImGui();
    void DeinitializeImGui();
    void ImGuiBeginFrame();
    void ImGuiDrawFrame();
    void UpdateImGuiViewport();
    void SwapBuffers();

    void SetVsync(bool bVsyncEnabled);

private:
    void* m_Window;

    void Initialize(const Window& contextWindow);
};

