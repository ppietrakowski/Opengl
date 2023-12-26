#pragma once

class GraphicsContext {
public:
    virtual ~GraphicsContext() = default;

    virtual void InitializeForImGui() = 0;
    virtual void DeinitializeImGui() = 0;
    virtual void ImGuiBeginFrame() = 0;
    virtual void ImGuiDrawFrame() = 0;
    virtual void UpdateImGuiViewport() = 0;
    virtual void SwapBuffers() = 0;

    virtual void SetVsync(bool vsync_enabled) = 0;
};

