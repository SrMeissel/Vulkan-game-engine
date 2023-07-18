#pragma once

#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>
#include <string>

namespace engine{

    class Window{
        public:
            Window(int w, int l, std::string name);
            ~Window();

            bool shouldClose(){
                return glfwWindowShouldClose(window);
            }

        private:

            void initWindow();
             
            const int width;
            const int height;

            std::string windowName;
            GLFWwindow *window;
    };
}