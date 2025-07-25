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
    ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
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

    ImGui::Begin("Objects");

    if (coder) {


                // === Light Controls ===
        static float lightPos[3] = {0.0f, 5.0f, 0.0f}; // Initial light position
        if (ImGui::SliderFloat3("Light Position", lightPos, -20.0f, 20.0f)) {
            if (renderer) renderer->setLightPosition(lightPos[0], lightPos[1], lightPos[2]);
        }

        ImGui::Separator(); // Visual separation from object controls

        // Add new object
        static int objectTypeIndex = 0; // 0 = sphere, 1 = box
        const char* objectTypes[] = {"Sphere", "Box"};

        static float pos[3] = {0, 0, 4};
        static float size[3] = {1.0f, 1.0f, 1.0f}; // For spheres, only size[0] is used
        static float color[3] = {1, 0, 0};
        static int operationIndex = 0; // 0 = union, 1 = intersection, 2 = subtract
        const char* operations[] = {"Smooth Union", "Intersection", "Smooth Subtract", "Union", "Subtract"};

        // Select object type
        ImGui::Combo("Object Type", &objectTypeIndex, objectTypes, IM_ARRAYSIZE(objectTypes));

        // Common inputs for all objects
        ImGui::InputFloat3("Position", pos);
        ImGui::ColorEdit3("Color", color);
        ImGui::Combo("Operation", &operationIndex, operations, IM_ARRAYSIZE(operations));

        // Inputs specific to the selected object type
        if (objectTypeIndex == 0) { // Sphere
            ImGui::SliderFloat("Radius", &size[0], 0.1f, 5.0f);
        } else if (objectTypeIndex == 1) { // Box
            ImGui::InputFloat3("Size (XYZ)", size);
        }

        // Add object button
        if (ImGui::Button("Add Object")) {
            std::string operation;
            if (operationIndex == 0) {
                operation = "smoothunion";
            } else if (operationIndex == 1) {
                operation = "intersection";
            } else if (operationIndex == 2) {
                operation = "smoothsubtract";
            }else if (operationIndex == 3) {
                operation = "union";
            } else if (operationIndex == 4) {
                operation = "subtract";
            }

            if (objectTypeIndex == 0) {
                // Add sphere
                coder->addSphere(pos[0], pos[1], pos[2], size[0], color[0], color[1], color[2], operation);
            } else if (objectTypeIndex == 1) {
                // Add box
                coder->addBox(pos[0], pos[1], pos[2], {size[0], size[1], size[2]}, color[0], color[1], color[2], operation);
            }

            if (renderer) renderer->pipelineDirty = true;
        }

        ImGui::Separator();

        // List and edit existing objects
        auto& objects = coder->getObjects();
        for (size_t i = 0; i < objects.size(); ++i) {
            ImGui::PushID((int)i);
            auto& obj = objects[i];
            bool changed = false;

            // Display object type
            ImGui::Text("Type: %s", obj.type.c_str());

            // Common inputs for all objects
            changed |= ImGui::InputFloat3("Pos", &obj.x);
            changed |= ImGui::ColorEdit3("Col", &obj.r);

            // Inputs specific to the object type
            if (obj.type == "sphere") {
                changed |= ImGui::SliderFloat("Radius", &obj.size[0], 0.1f, 5.0f);
            } else if (obj.type == "box") {
                changed |= ImGui::InputFloat3("Size (XYZ)", obj.size.data());
            }

            // Operation selection
            int currentOperationIndex;
            if (obj.operation == "smoothunion") {
                currentOperationIndex = 0;
            } else if (obj.operation == "intersection") {
                currentOperationIndex = 1;
            } else if (obj.operation == "smoothsubtract") {
                currentOperationIndex = 2;
            }else if (obj.operation == "union") {
                currentOperationIndex = 3;
            } else if (obj.operation == "subtract") {
                currentOperationIndex = 4;
            } else {
                currentOperationIndex = 0; // Default to union
            }

            if (ImGui::Combo("Operation", &currentOperationIndex, operations, IM_ARRAYSIZE(operations))) {
                if (currentOperationIndex == 0) {
                    obj.operation = "smoothunion";
                } else if (currentOperationIndex == 1) {
                    obj.operation = "intersection";
                } else if (currentOperationIndex == 2) {
                    obj.operation = "smoothsubtract";
                }else if (currentOperationIndex == 3) {
                    obj.operation = "union";
                } else if (currentOperationIndex == 4) {
                    obj.operation = "subtract";
                }
                changed = true;
            }

            // Remove object button
            if (ImGui::Button("Remove")) {
                objects.erase(objects.begin() + i);
                if (renderer) renderer->pipelineDirty = true;
                ImGui::PopID();
                break;
            }

            if (changed) {
                if (renderer) renderer->pipelineDirty = true;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if (ImGui::Button("Clear All")) {
            coder->clearObjects();
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

    coder->addBox(0, 0, 0, {1.0f, 1.0f, 1.0f}, 1.0f, 1.0f, 1.0f, "union");// Add a default box for testing
    renderer->pipelineDirty = true;
}