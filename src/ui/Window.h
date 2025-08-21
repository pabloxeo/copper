#ifndef WINDOW_H
#define WINDOW_H

#include <webgpu/webgpu_glfw.h>
#include <GLFW/glfw3.h>

class Renderer;

class Window {
public:

    bool Initialize(Renderer *renderer);

    GLFWwindow* getWindow() { return window; }

    uint32_t getWindowWidth() const { return windowWidth; }
    uint32_t getWindowHeight() const { return windowHeight; }

    void setWindowSize(uint32_t width, uint32_t height) {
        windowWidth = width;
        windowHeight = height;
    }

    void Destroy() {
        if (window != nullptr) {
            glfwDestroyWindow(window);
            window = nullptr; // Reset the pointer to avoid dangling references
        }
    }

    void SetWindowUserPointer(void* pointer);
    static void FramebufferSizeCallback(GLFWwindow* window, __attribute_maybe_unused__ int width, __attribute_maybe_unused__ int height);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void MouseMoveCallback(GLFWwindow *window, double xpos, double ypos);

private:
    GLFWwindow* window = nullptr;
    uint32_t windowWidth;
    uint32_t windowHeight;

};


#endif //WINDOW_H