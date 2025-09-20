#pragma once

#include "keyMap.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>
#include <string>
#include <bitset>

#include <vulkan/vulkan.h>

struct Window {
    virtual ~Window() {}
    virtual void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) = 0;
    virtual bool wasWindowResized() = 0;
    virtual void resizeWindow(int w, int h) = 0;
    virtual void resetWindowResizedFlag() = 0;
    virtual bool shouldClose() = 0;

    virtual void setKeyDown(Key key) = 0;
    virtual void setKeyUp(Key key) = 0;
    virtual bool isKeyDown(Key key) = 0;

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
        void resizeWindow(int w, int h) override {
            frameBufferResized = true;
        }
        void resetWindowResizedFlag() override {
            frameBufferResized = false;
        }

        virtual void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) override;
        virtual bool isKeyDown(Key key) override;

        virtual void setKeyDown(Key key) override {};
        virtual void setKeyUp(Key key) override {};

    private:
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);
        void initWindow();
            
        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowName;
        GLFWwindow *window;
};

class Window_win final : public Window {
    public:
        Window_win(int w, int h, HWND handle);

        virtual void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) override;
        
        bool wasWindowResized() override {
            return frameBufferResized;
        }
        void resizeWindow(int w, int h) override {
            frameBufferResized = true;
            width = w;
            height = h;
        }
        void resetWindowResizedFlag() override {
            frameBufferResized = false;
         }

        bool shouldClose() override {
            return false;
        }

        VkExtent2D getExtent() override {
            return {static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        bool isKeyDown(Key key) override;
        void setKeyDown(Key key) override;
        void setKeyUp(Key key) override;

    private:
        HWND handle;
        HINSTANCE instance_win = NULL;

        std::bitset<static_cast<size_t>(Key::KeyCount)> keyState;

        int width;
        int height;

        bool frameBufferResized = false;
};
