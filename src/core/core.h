#ifndef CORE_H
#define CORE_H

#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_glfw.h>
#include <ui/window.h>
#include "./renderer.h"

using namespace wgpu;


class Core {
public:
    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();

private:
    // We put here all the variables that are shared between init and main loop
    Window window;
    Renderer renderer;
    float time = 0.0f;
};



#endif //CORE_H