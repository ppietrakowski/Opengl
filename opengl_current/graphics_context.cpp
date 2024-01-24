#include "graphics_context.h"
#include "error_macros.h"

#include <GL/glew.h>

#include "Imgui/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

GraphicsContext::GraphicsContext(GLFWwindow* contextWindow) :
    m_Window{contextWindow}
{
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);
    GLenum errorCode = glewInit();
    CRASH_EXPECTED_TRUE_MSG(errorCode == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(errorCode)));
}

constexpr const char* kGlslVersion = "#version 430 core";

void GraphicsContext::InitializeForImGui()
{
    ImGui_ImplOpenGL3_Init(kGlslVersion);
}

void GraphicsContext::DeinitializeImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
}

void GraphicsContext::ImGuiBeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
}

void GraphicsContext::ImGuiDrawFrame()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsContext::UpdateImGuiViewport()
{

}

void GraphicsContext::SwapBuffers()
{
    glfwSwapBuffers(m_Window);
}

void GraphicsContext::SetVsync(bool bVsyncEnabled)
{
    glfwSwapInterval(bVsyncEnabled ? 1 : 0);
}
