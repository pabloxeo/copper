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

    // Getter functions
    float getWindowWidth() const { return windowWidth; }
    float getWindowHeight() const { return windowHeight; }

    // Setter functions
    void setWindowSize(float width, float height) {
        windowWidth = width;
        windowHeight = height;
    }

    // Function for updating buffers like light position or window size
    void UpdateBuffers();

    // Reconfigure surface on window resize
    void ReconfigureSurface();

private:
    TextureView GetNextSurfaceTextureView();

    // Substep of Initialize() that creates the render pipeline
    void InitializePipeline();

    TextureView CreateMSAATargetView();

    void Present();

    void Render();

    void Update();

    void ProcessInput();

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
    Buffer uFovBuffer;
    Buffer uLightPositionBuffer;
    Buffer uViewPositionBuffer;
    Buffer uWindowSizeBuffer;
    float windowWidth;
    float windowHeight;
    BindGroupLayout bindGroupLayout; // Bind group layout for uTime

    float lightPos[3] = {0.0f, 5.0f, 0.0f};  // Initialize light position
    // ... other members ...
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
        app->setWindowSize(static_cast<float>(width), static_cast<float>(height));
        glViewport(0, 0, width, height);
        app->ReconfigureSurface();  // Ensure the surface is reconfigured after resize
    }

    static void KeyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
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
