#pragma once

#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>
#include <string>

struct Window {
    virtual ~Window() {}
    virtual void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) = 0;
    virtual bool wasWindowResized() = 0;
    virtual void resetWindowResizedFlag() = 0;
    virtual bool shouldClose() = 0;
    virtual bool isKeyDown() = 0;
    
    //dont keep this
    virtual void* getWindowHandle() = 0;

    //virtual void* getWindowHandle() = 0;
    virtual VkExtent2D getExtent() = 0;
};

class Window_GLFW final : public Window {
    public:
        Window_GLFW(int w, int l, std::string name);
        ~Window_GLFW();

        bool shouldClose() override {
            return glfwWindowShouldClose(window);
        }
        VkExtent2D getExtent() override {
            return {static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }
        bool wasWindowResized() override {
            return frameBufferResized;
        }
        void resetWindowResizedFlag() override {
            frameBufferResized = false;
        }
        GLFWwindow* getGLFWwindow() const {return window;}
        void* getWindowHandle() override {return getGLFWwindow(); }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        virtual bool isKeyDown() override;

    private:
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);
        void initWindow();
            
        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowName;
        GLFWwindow *window;
};

