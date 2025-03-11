//
// Created by pabloxeo on 3/11/25.
//

#include "Window.h"

bool Window::Initialize() {
	// Open window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    windowWidth = videoMode->width;
    windowHeight = videoMode->height;

    window = glfwCreateWindow(windowWidth, windowHeight, "Copper", nullptr, nullptr);
    glfwSetWindowAspectRatio(window, 16, 9);
    glfwSetWindowUserPointer(window, this); // Set the user pointer to access App instance
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    int tempWidth, tempHeight;
    glfwGetFramebufferSize(window, &tempWidth, &tempHeight);

    // Convert to float and assign
    windowWidth = static_cast<float>(tempWidth);
    windowHeight = static_cast<float>(tempHeight);    
    
    return (window != nullptr);
}