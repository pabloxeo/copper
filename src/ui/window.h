#include <GLFW/glfw3.h>
#include <functional>
#include "../core/renderer.h"

#ifndef COPPER_WINDOW_H
#define COPPER_WINDOW_H


class Window {
public:

    bool Initialize();

    GLFWwindow* getWindow() { return window; }

    // Getter functions
    uint32_t getWindowWidth() const { return windowWidth; }
    uint32_t getWindowHeight() const { return windowHeight; }

    // Setter functions
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
    static void glfwSetMouseButtonCallback(GLFWwindow *window, GLFWmousebuttonfun callback);

private:
    GLFWwindow* window = nullptr;
    uint32_t windowWidth;
    uint32_t windowHeight;

};


#endif //COPPER_WINDOW_H