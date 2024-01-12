#include "graphics_context.h"
#include "error_macros.h"

#include <GL/glew.h>

#include "Imgui/imgui_impl_glfw.h"
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

void GraphicsContext::InitializeForImGui()
{
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    const char* glslVersion = "#version 430 core";
    ImGui_ImplOpenGL3_Init(glslVersion);
}

void GraphicsContext::DeinitializeImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GraphicsContext::ImGuiBeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void GraphicsContext::ImGuiDrawFrame()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsContext::UpdateImGuiViewport()
{
    ImGuiIO& io = ImGui::GetIO();

    // update viewport if enabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* currentContextWindow = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(currentContextWindow);
    }
}

void GraphicsContext::SwapBuffers()
{
    glfwSwapBuffers(m_Window);
}

void GraphicsContext::SetVsync(bool bVsyncEnabled)
{
    glfwSwapInterval(bVsyncEnabled ? 1 : 0);
}
