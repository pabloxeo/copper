//
// Created by pablo on 20/07/2024.


#ifndef APP_H
#define APP_H

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include "../ui/Window.h"
#include "../renderer/Renderer.h"

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
    // We put here all the variables that are shared between init and main loop
    Window window;
    Renderer renderer;
    
    void OnWindowResize(int width, int height) {
        // Notify the renderer to reconfigure its surface and update any buffers
        renderer.ReconfigureSurface(static_cast<float>(width), static_cast<float>(height));
        renderer.UpdateWindow();
    }

/*
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
*/
};



#endif //APP_H

