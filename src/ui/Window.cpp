#include "./Window.h"
#include "../core/Renderer.h"

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
    glfwSetWindowUserPointer(window, this);
    
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    int tempWidth, tempHeight;
    glfwGetFramebufferSize(window, &tempWidth, &tempHeight);

    windowWidth = static_cast<float>(tempWidth);
    windowHeight = static_cast<float>(tempHeight);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    return (window != nullptr);
}

void Window::SetWindowUserPointer(void* pointer) {
    glfwSetWindowUserPointer(window, pointer);
}

void Window::FramebufferSizeCallback(GLFWwindow* window,__attribute_maybe_unused__ int width,__attribute_maybe_unused__ int height) {
    auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer != nullptr) {
        renderer->OnResize();
    }
}

void Window::MouseButtonCallback(GLFWwindow *window, int button, int action,__attribute_maybe_unused__ int mods) {
    auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer != nullptr && !ImGui::GetIO().WantCaptureMouse && action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        renderer->OnMouseButton(button, action);
    }
}