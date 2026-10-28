#include <Engine.Render/WidgetRenderDevice.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace rw::widgets
{
void UIContext::preRenderFrame()
{
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    // if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    // {
    //     ImGui_ImplGlfw_Sleep(10);
    //     continue;
    // }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIContext::renderFrame()
{
    ImGui::DockSpaceOverViewport();

    for(UIPanel *surface : m_Collection)
    {
        surface->drawFrame();
    }
}

void UIContext::postRenderFrame()
{
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    coreRenderGetFramebufferSize(&display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    coreRenderSwapBuffers();
}

UIContext::UIContext()
{}

void UIContext::getViewport(u32* x, u32* y, u32* w, u32* h) const
{

}

void UIContext::setViewport(u32 x, u32 y, u32 w, u32 h)
{
    // glViewport(x, y, w, h);
}

void UIContext::clearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void UIContext::clearDepth(float d)
{
    glClearDepth(d);
}

void UIContext::clearStencil(u32 v)
{
    glClearStencil(v);
}

void UIContext::addSurface(UIPanel *surface)
{
    m_Collection.push_back(surface);
}
}