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
    Buffer uLightPositionBuffer;
    Buffer uViewPositionBuffer;

    BindGroupLayout bindGroupLayout; // Bind group layout for uTime

    float lightPos[3] = {0.0f, 5.0f, 0.0f};  // Initialize light position
    // ... other members ...

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_UP:
                    app->lightPos[2] += 0.1f; // Move light up
                    break;
                case GLFW_KEY_DOWN:
                    app->lightPos[2] -= 0.1f; // Move light down
                    break;
                case GLFW_KEY_LEFT:
                    app->lightPos[0] -= 0.1f; // Move light left
                    break;
                case GLFW_KEY_RIGHT:
                    app->lightPos[0] += 0.1f; // Move light right
                    break;
            }
        }
    }
};



#endif //APP_H
