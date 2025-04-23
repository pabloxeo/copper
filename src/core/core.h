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
/*
    static void KeyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
        Core* core = static_cast<Core*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_UP:
                    core->lightPos[2] += 0.1f; // Move light up
                    break;
                case GLFW_KEY_DOWN:
                    core->lightPos[2] -= 0.1f; // Move light down
                    break;
                case GLFW_KEY_LEFT:
                    core->lightPos[0] -= 0.1f; // Move light left
                    break;
                case GLFW_KEY_RIGHT:
                    core->lightPos[0] += 0.1f; // Move light right
                    break;
            }
        }
    }
*/
};



#endif //CORE_H