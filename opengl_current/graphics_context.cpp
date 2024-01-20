#include "graphics_context.h"
#include "error_macros.h"

#include <GL/glew.h>

#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

GraphicsContext::GraphicsContext(GLFWwindow* context_window) :
    window_{context_window} {
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    GLenum error_code = glewInit();
    CRASH_EXPECTED_TRUE_MSG(error_code == GLEW_OK, reinterpret_cast<const char*>(glewGetErrorString(error_code)));
}

constexpr const char* kGlslVersion = "#version 430 core";

void GraphicsContext::InitializeForImGui() {
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(kGlslVersion);
}

void GraphicsContext::DeinitializeImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GraphicsContext::ImGuiBeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void GraphicsContext::ImGuiDrawFrame() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsContext::UpdateImGuiViewport() {
    ImGuiIO& io = ImGui::GetIO();

    // update viewport if enabled
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* currentContextWindow = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(currentContextWindow);
    }
}

void GraphicsContext::SwapBuffers() {
    glfwSwapBuffers(window_);
}

void GraphicsContext::SetVsync(bool vsync_enabled) {
    glfwSwapInterval(vsync_enabled ? 1 : 0);
}
