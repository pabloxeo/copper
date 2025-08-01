//
// Created by pablo on 20/07/2024.
//


#include "./Core.h"

bool Core::Initialize() {

    if (!window.Initialize()) return false;

    if(!renderer.Init(&window)) return false;
    
    return true;
}

void Core::Terminate() {
    renderer.Release();
    window.Destroy();
    glfwTerminate();
}

void Core::MainLoop()
{   
    glfwPollEvents();
    renderer.Render();
}

bool Core::IsRunning() {
    return !glfwWindowShouldClose(window.getWindow());
}