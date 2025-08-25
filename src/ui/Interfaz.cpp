#include "../core/Renderer.h"
#include "./Interfaz.h"
#include <ImGuiFileDialog.h>


bool Interfaz::initGui(Window window, wgpu::Device device, wgpu::TextureFormat format) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
    ImGui_ImplWGPU_InitInfo initInfo = {};
    initInfo.Device = device.Get();
    initInfo.DepthStencilFormat = WGPUTextureFormat::WGPUTextureFormat_Depth24Plus;
    initInfo.NumFramesInFlight = 3;
    initInfo.RenderTargetFormat = (WGPUTextureFormat)format;
    
    ImGui_ImplWGPU_Init(&initInfo);
    return true;
}

void Interfaz::terminateGui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
}

void Interfaz::updateGui(wgpu::RenderPassEncoder renderPass) {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    

    

    if (coder) {
        ImGui::Begin("Selected Object");
        int id = coder->getSelectedObjectId();
        if (id >= 0) {
            Object& selectedObject = coder->getSelectedObject();

            ImGui::SliderFloat("X", &selectedObject.x, -10.0f, 10.0f);
            ImGui::SliderFloat("Y", &selectedObject.y, -10.0f, 10.0f);
            ImGui::SliderFloat("Z", &selectedObject.z, -10.0f, 10.0f);
            ImGui::ColorEdit3("Color", &selectedObject.r);
            
            if (selectedObject.type == "sphere") {
                ImGui::SliderFloat("Radius", &selectedObject.size[0], 0.1f, 5.0f);
            } else if (selectedObject.type == "box") {
                ImGui::DragFloat3("Size (XYZ)", selectedObject.size.data());
            } else if (selectedObject.type == "cone") {
                ImGui::DragFloat2("Base Radius, Height", selectedObject.size.data(), 0.1f, 0.1f, 5.0f);
                ImGui::DragFloat("Top Radius", &selectedObject.size[2], 0.1f, 0.0f, 5.0f);
            } else if (selectedObject.type == "cylinder") {
                ImGui::DragFloat("Radius", &selectedObject.size[0], 0.1f, 0.1f, 5.0f);
                ImGui::DragFloat("Height", &selectedObject.size[1], 0.1f, 0.1f, 10.0f);
            }


            if (ImGui::Button("Delete Object")) {
                coder->deleteObject(id);
                coder->setSelectedObjectId(-1);
                if (renderer) renderer->pipelineDirty = true;
            }
        } else {
            ImGui::Text("No object selected");
        }
        ImGui::End();

        ImGui::Begin("Objects");


        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        static float lightPos[3] = {0.0f, 5.0f, 0.0f}; // Initial light position
        if (ImGui::SliderFloat3("Light Position", lightPos, -20.0f, 20.0f)) {
            if (renderer) renderer->setLightPosition(lightPos[0], lightPos[1], lightPos[2]);
        }

        ImGui::Separator();

        if(ImGui::Checkbox("Show Floor", &renderer->floor)) {
            renderer->pipelineDirty = true; // Mark pipeline as dirty to update the floor rendering
        }

        ImGui::Separator();

        static int objectTypeIndex = 0;
        const char* objectTypes[] = {"Sphere", "Box", "Cone", "Cylinder"};

        static float pos[3] = {0.0f, 0.0f, 0.0f};
        static float size[3] = {1.0f, 1.0f, 1.0f};
        static float color[3] = {0.0f, 0.3f, 1.0f};
        static int operationIndex = 0;
        const char* operations[] = {"Smooth Union", "Intersection", "Smooth Subtract", "Union", "Subtract"};

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
        }else if (objectTypeIndex == 2) { // Cone
            ImGui::InputFloat2("Base Radius, Top Radius", &size[1]);
            ImGui::InputFloat("Height", &size[0]);
        } else if (objectTypeIndex == 3) { // Cylinder
            ImGui::InputFloat("Radius", &size[0]);
            ImGui::InputFloat("Height", &size[1]);
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
            }else if (objectTypeIndex == 2) {
                // Add cone
                coder->addCone(pos[0], pos[1], pos[2], {size[0], size[1], size[2]}, color[0], color[1], color[2], operation);
            } else if (objectTypeIndex == 3) {
                // Add cylinder
                coder->addCylinder(pos[0], pos[1], pos[2], {size[0], size[1]}, color[0], color[1], color[2], operation);
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
            std::string name = obj.type + std::to_string(obj.id);
            ImGui::Text("Name: %s", name.c_str());

            // Operation selection
            int currentOperationIndex = obj.operation == "smoothunion" ? 0 :
                                        obj.operation == "intersection" ? 1 :
                                        obj.operation == "smoothsubtract" ? 2 :
                                        obj.operation == "union" ? 3 : 4;

            if (ImGui::Combo("Operation", &currentOperationIndex, operations, IM_ARRAYSIZE(operations))) {
                obj.operation = currentOperationIndex == 0 ? "smoothunion" :
                                    currentOperationIndex == 1 ? "intersection" :
                                    currentOperationIndex == 2 ? "smoothsubtract" :
                                    currentOperationIndex == 3 ? "union" : "subtract";
                //printf("Operation changed to: %s\n", obj.operation.c_str());
                changed = true;
            }

            // Remove object button
            if (ImGui::Button("Remove")) {
                if (coder->getSelectedObjectId() == obj.id) coder->setSelectedObjectId(-1);
                if (renderer) renderer->pipelineDirty = true;
                objects.erase(objects.begin() + i);
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
            if (renderer) renderer->pipelineDirty = true;
            coder->setSelectedObjectId(-1);
            coder->clearObjects();
        }

        ImGui::Separator();

        // === Save/Load Section with File Dialog ===
        if (ImGui::Button("Save Scene As...")) {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileSave", "Save Scene", ".copper", ".");
        }

        ImGui::SameLine();
        if (ImGui::Button("Load Scene...")) {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileLoad", "Load Scene", ".copper", ".");
        }

        // Display the file dialogs
        if (ImGuiFileDialog::Instance()->Display("ChooseFileSave")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (coder->saveScene(filePath)) {
                    printf("Scene saved successfully to: %s\n", filePath.c_str());
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileLoad")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                if (coder->loadScene(filePath)) {
                    if (renderer) renderer->pipelineDirty = true;
                    printf("Scene loaded successfully from: %s\n", filePath.c_str());
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

    }

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass.Get());
}

void Interfaz::setCoderAndRenderer(Coder* c, Renderer* r) {
    coder = c;
    renderer = r;
    //coder->addTest(); // Add test objects for demonstration
}