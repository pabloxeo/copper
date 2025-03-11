//
// Created by pablo on 20/07/2024.
//

#include "App.h"
#include "../ui/Window.h"
#include "../renderer/Renderer.h"


using namespace wgpu;



bool App::Initialize() {
    // Open window first add window.h
    if (!window.Initialize()) {
        return false;
    }    

    window.SetResizeCallback([this](int width, int height) {
        this->OnWindowResize(width, height);
    });

    if (!renderer.Initialize(window)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    renderer.UpdateWindow();

    return true;
}

void App::Terminate() {
    renderer.release();
    glfwDestroyWindow(window.GetWindow());
    glfwTerminate();
}

void App::MainLoop()
{
    renderer.RenderFrame();
    renderer.UpdateBuffers();
}

bool App::IsRunning() {
    return !glfwWindowShouldClose(window.GetWindow());
}