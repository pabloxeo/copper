//
// Created by pabloxeo on 3/11/25.
//

#include <GLFW/glfw3.h>
#include <functional>

#ifndef COPPER_WINDOW_H
#define COPPER_WINDOW_H


class Window {
public:
    using ResizeCallback = std::function<void(int, int)>;

    bool Initialize();

    GLFWwindow* GetWindow() { return window; }

    // Getter functions
    float getWindowWidth() const { return windowWidth; }
    float getWindowHeight() const { return windowHeight; }

    // Setter functions
    void setWindowSize(float width, float height) {
        windowWidth = width;
        windowHeight = height;
    }

    void SetResizeCallback(ResizeCallback callback) {
        resizeCallback = callback;
    }

private:
    GLFWwindow* window = nullptr;
    float windowWidth;
    float windowHeight;
    ResizeCallback resizeCallback;

    // Static callback that GLFW calls when the window is resized.
    // It retrieves the Window instance and calls the stored callback.
    static void FramebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        self->setWindowSize(newWidth, newHeight);
        glViewport(0, 0, newWidth, newHeight);
        if (self && self->resizeCallback) {
            self->resizeCallback(newWidth, newHeight);
        }
    }

};


#endif //COPPER_WINDOW_H
