//
// Created by pablo on 20/07/2024.


#ifndef APP_H
#define APP_H

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

using namespace wgpu;


class App {
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
    TextureView GetNextSurfaceTextureView();

    // Substep of Initialize() that creates the render pipeline
    void InitializePipeline();

private:
    // We put here all the variables that are shared between init and main loop
    GLFWwindow *window;
    Device device;
    Queue queue;
    Surface surface;
    std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;
    TextureFormat surfaceFormat = TextureFormat::Undefined;
    RenderPipeline pipeline;
    BindGroup bindGroup;         // Bind group for uniform bindings
    Buffer uTimeBuffer;          // Buffer for the uTime uniform
    Buffer uMouseBuffer;
    BindGroupLayout bindGroupLayout; // Bind group layout for uTime
};



#endif //APP_H
