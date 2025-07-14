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
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = true; // Enable mouse cursor drawing

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window.GetWindow(), true);
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
        static int operationIndex = 0; // 0 = union, 1 = intersection, 2 = subtract
        const char* operations[] = {"Union", "Intersection", "Subtract"};

        if (ImGui::InputFloat3("Position", pos) |
            ImGui::SliderFloat("Radius", &radius, 0.1f, 5.0f) |
            ImGui::ColorEdit3("Color", color) |
            ImGui::Combo("Operation", &operationIndex, operations, IM_ARRAYSIZE(operations))) {
            // Optionally preview new sphere
        }

        if (ImGui::Button("Add Sphere")) {
            std::string operation;
            if (operationIndex == 0) {
                operation = "union";
            } else if (operationIndex == 1) {
                operation = "intersection";
            } else {
                operation = "subtract";
            }
            coder->addSphere(pos[0], pos[1], pos[2], radius, color[0], color[1], color[2], operation);
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

            int currentOperationIndex;
            if (s.operation == "union") {
                currentOperationIndex = 0;
            } else if (s.operation == "intersection") {
                currentOperationIndex = 1;
            } else {
                currentOperationIndex = 2;
            }

            if (ImGui::Combo("Operation", &currentOperationIndex, operations, IM_ARRAYSIZE(operations))) {
                if (currentOperationIndex == 0) {
                    s.operation = "union";
                } else if (currentOperationIndex == 1) {
                    s.operation = "intersection";
                } else {
                    s.operation = "subtract";
                }
                changed = true;
            }

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