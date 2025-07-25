//
// Created by pablo on 20/07/2024.
//

#include "./core.h"
#include "../ui/window.h"
#include "./renderer.h"
#include <iostream>

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