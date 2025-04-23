#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include "./controls.h"
#include <iostream>

bool Controls::initGui(Window window, wgpu::Device device, wgpu::TextureFormat format) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window.GetWindow(), true);
    ImGui_ImplWGPU_InitInfo initInfo = {};
    initInfo.Device = device.Get();
    initInfo.DepthStencilFormat = WGPUTextureFormat::WGPUTextureFormat_Depth24Plus;
    initInfo.NumFramesInFlight = 3;
    initInfo.RenderTargetFormat = (WGPUTextureFormat)format;
    
    ImGui_ImplWGPU_Init(&initInfo);
    return true;
}

void Controls::terminateGui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
}

void Controls::updateGui(wgpu::RenderPassEncoder renderPass) {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    static float f = 0.0f;
    static int counter = 0;
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGui::Begin("Hello, world!");                               
    ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

    if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
    // Draw the UI
    ImGui::EndFrame();
    // Convert the UI defined above into low-level drawing commands
    ImGui::Render();
    // Execute the low-level drawing commands on the WebGPU backend
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass.Get());
}