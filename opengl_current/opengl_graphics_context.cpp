#include "opengl_graphics_context.h"

#include "error_macros.h"

OpenGlGraphicsContext::OpenGlGraphicsContext(GLFWwindow* contextWindow) :
    Window{contextWindow}
{
    glfwMakeContextCurrent(Window);
    glfwSwapInterval(1);
    GLenum errorCode = glewInit();
    CRASH_EXPECTED_TRUE_MSG(errorCode == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(errorCode)));
}

void OpenGlGraphicsContext::InitializeForImGui()
{
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    const char* glslVersion = "#version 430 core";
    ImGui_ImplOpenGL3_Init(glslVersion);
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
        GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupCurrentContext);
    }
}

void OpenGlGraphicsContext::SwapBuffers()
{
    glfwSwapBuffers(Window);
}

void OpenGlGraphicsContext::SetVsync(bool vsync_enabled)
{
    glfwSwapInterval(vsync_enabled ? 1 : 0);
}
