#pragma once

struct GLFWwindow;

class GraphicsContext {
public:
    GraphicsContext(GLFWwindow* context_window);
    ~GraphicsContext() = default;

public:
    void InitializeForImGui();
    void DeinitializeImGui();
    void ImGuiBeginFrame();
    void ImGuiDrawFrame();
    void UpdateImGuiViewport();
    void SwapBuffers();

    void SetVsync(bool vsync_enabled);

private:
    GLFWwindow* window_;
};

