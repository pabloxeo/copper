#include "./window.h"
#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>

bool Window::Initialize() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    windowWidth = videoMode->width;
    windowHeight = videoMode->height;

    window = glfwCreateWindow(windowWidth, windowHeight, "Copper", nullptr, nullptr);
    glfwSetWindowAspectRatio(window, 16, 9);
    glfwSetWindowUserPointer(window, this); // Set the user pointer to access the Window instance
    
    // Set the framebuffer size callback
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    int tempWidth, tempHeight;
    glfwGetFramebufferSize(window, &tempWidth, &tempHeight);

    // Convert to float and assign
    windowWidth = static_cast<float>(tempWidth);
    windowHeight = static_cast<float>(tempHeight);

    return (window != nullptr);
}

void Window::SetWindowUserPointer(void* pointer) {
    glfwSetWindowUserPointer(window, pointer);
}

void Window::FramebufferSizeCallback(GLFWwindow* window,__attribute_maybe_unused__ int width,__attribute_maybe_unused__ int height) {
    // Get the renderer pointer and call onResize
    auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer != nullptr) {
        renderer->OnResize();
    }
}