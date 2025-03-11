//
// Created by pabloxeo on 3/11/25.
//

#ifndef COPPER_WINDOW_H
#define COPPER_WINDOW_H


class window {
    public:
        bool Initialize(const char* title) {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            // Create window
            window = glfwCreateWindow(width, height, title, nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetKeyCallback(window, KeyCallback);
            glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

            // Update width/height if necessary
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            width = static_cast<float>(fbWidth);
            height = static_cast<float>(fbHeight);

            return (window != nullptr);
        }

        GLFWwindow* GetWindow() { return window; }

        // Getter functions
        float getWindowWidth() const { return windowWidth; }
        float getWindowHeight() const { return windowHeight; }

        // Setter functions
        void setWindowSize(float width, float height) {
            windowWidth = width;
            windowHeight = height;
        }


    private:
            GLFWwindow* window = nullptr;
            float width = 800.0f;
            float height = 600.0f;


};


#endif //COPPER_WINDOW_H
