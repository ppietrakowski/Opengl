#include "opengl_graphics_context.h"

#include "error_macros.h"

OpenGlGraphicsContext::OpenGlGraphicsContext(GLFWwindow* contextWindow) :
    window_{contextWindow}
{
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    GLenum error_code = glewInit();
    CRASH_EXPECTED_TRUE_MSG(error_code == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(error_code)));
}

void OpenGlGraphicsContext::InitializeForImGui()
{
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    const char* glsl_version = "#version 430 core";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void OpenGlGraphicsContext::DeinitializeImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void OpenGlGraphicsContext::ImGuiBeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void OpenGlGraphicsContext::ImGuiDrawFrame()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGlGraphicsContext::UpdateImGuiViewport()
{
    ImGuiIO& io = ImGui::GetIO();

    // update viewport if enabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* current_context_window = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(current_context_window);
    }
}

void OpenGlGraphicsContext::SwapBuffers()
{
    glfwSwapBuffers(window_);
}

void OpenGlGraphicsContext::SetVsync(bool vsync_enabled)
{
    glfwSwapInterval(vsync_enabled ? 1 : 0);
}
