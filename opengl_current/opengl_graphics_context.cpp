#include "opengl_graphics_context.h"

#include "error_macros.h"

OpenGlGraphicsContext::OpenGlGraphicsContext(GLFWwindow* context_window):
    context_window_{ context_window } {
    glfwMakeContextCurrent(context_window_);
    glfwSwapInterval(1);
    GLenum error_code = glewInit();
    CRASH_EXPECTED_TRUE_MSG(error_code == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(error_code)));
}

void OpenGlGraphicsContext::InitializeForImGui() {
    ImGui_ImplGlfw_InitForOpenGL(context_window_, true);
    const char* glsl_version = "#version 430 core";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void OpenGlGraphicsContext::DeinitializeImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void OpenGlGraphicsContext::ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void OpenGlGraphicsContext::ImGuiDrawFrame() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGlGraphicsContext::UpdateImGuiViewport() {
    ImGuiIO& io = ImGui::GetIO();

    // update viewport if enabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void OpenGlGraphicsContext::SwapBuffers() {
    glfwSwapBuffers(context_window_);
}

void OpenGlGraphicsContext::SetVsync(bool vsync_enabled) {
    glfwSwapInterval(vsync_enabled ? 1 : 0);
}
