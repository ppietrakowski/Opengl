#pragma once

struct GLFWwindow;

class GraphicsContext
{
public:
    GraphicsContext(GLFWwindow* contextWindow);
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
    GLFWwindow* m_Window;
};

