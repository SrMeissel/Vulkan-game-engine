#include "windowManager.hpp"


#include <stdexcept>

Window_GLFW::Window_GLFW(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
    initWindow();
}

Window_GLFW::~Window_GLFW() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window_GLFW::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
}

void Window_GLFW::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
    if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
}

void Window_GLFW::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto newWindow = reinterpret_cast<Window_GLFW *>(glfwGetWindowUserPointer(window));
    newWindow->frameBufferResized = true;
    newWindow->width = width;
    newWindow->height = height;
}

 bool Window_GLFW::isKeyDown() {
    return false;
 }