#pragma once

class Window;

class GraphicsContext
{
public:
    GraphicsContext(const Window& contextWindow);
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
};

