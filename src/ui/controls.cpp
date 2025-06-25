#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <iostream>
#include "./controls.h"
#include "../core/coder.h"
#include "./window.h"

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

    ImGui::Begin("Spheres");

    if (coder) {
        // Add new sphere
        static float pos[3] = {0, 0, 4};
        static float radius = 1.0f;
        static float color[3] = {1, 0, 0};
        if (ImGui::InputFloat3("Position", pos) |
            ImGui::SliderFloat("Radius", &radius, 0.1f, 5.0f) |
            ImGui::ColorEdit3("Color", color)) {
            // Optionally preview new sphere
        }
        if (ImGui::Button("Add Sphere")) {
            coder->addSphere(pos[0], pos[1], pos[2], radius, color[0], color[1], color[2]);
            if (renderer) renderer->pipelineDirty = true;
        }
        ImGui::Separator();

        // List and edit existing spheres
        auto& spheres = coder->getSpheres();
        for (size_t i = 0; i < spheres.size(); ++i) {
            ImGui::PushID((int)i);
            auto& s = spheres[i];
            bool changed = false;
            changed |= ImGui::InputFloat3("Pos", &s.x);
            changed |= ImGui::SliderFloat("Rad", &s.radius, 0.1f, 5.0f);
            changed |= ImGui::ColorEdit3("Col", &s.r);
            if (ImGui::Button("Remove")) {
                spheres.erase(spheres.begin() + i);
                coder->generateShaderCode();
                if (renderer) renderer->pipelineDirty = true;
                ImGui::PopID();
                break;
            }
            if (changed) {
                coder->generateShaderCode();
                if (renderer) renderer->pipelineDirty = true;
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        if (ImGui::Button("Clear All")) {
            coder->clearSpheres();
            if (renderer) renderer->pipelineDirty = true;
        }
    }

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass.Get());
}

void Controls::setCoderAndRenderer(Coder* c, Renderer* r) {
    coder = c;
    renderer = r;
}