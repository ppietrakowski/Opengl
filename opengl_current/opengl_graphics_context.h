#pragma once

#include "graphics_context.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_impl_glfw.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class OpenGlGraphicsContext : public IGraphicsContext
{
public:
    OpenGlGraphicsContext(GLFWwindow* contextWindow);

public:
    void InitializeForImGui() override;
    void DeinitializeImGui() override;
    void ImGuiBeginFrame() override;
    void ImGuiDrawFrame() override;
    void UpdateImGuiViewport() override;
    void SwapBuffers() override;
    void SetVsync(bool bVsyncEnabled) override;

private:
    GLFWwindow* Window;
};

