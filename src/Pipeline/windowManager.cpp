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

//TODO: Finish.
bool Window_GLFW::isKeyDown() {
    return false;
}

Window_win::Window_win(int w, int h, HWND handle) : width{w}, height{h}, handle{handle} {
    instance_win = (HINSTANCE)GetWindowLongPtr(handle, GWLP_HINSTANCE);

    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
}

void Window_win::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd      = handle;
    createInfo.hinstance = instance_win;
    vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, surface);
}