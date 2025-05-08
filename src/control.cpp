#include "engineControl.h"

#include "Pipeline/windowManager.hpp"
#include "Pipeline/Renderer.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

typedef renderer::Renderer c_renderer;

#ifdef __cplusplus
extern "C" {
#endif

static Window* window = nullptr;  
static c_renderer* renderer = nullptr;

WindowHandle createEngine() {
    window = new Window{1280, 720, "Hello there"};
    renderer = new renderer::Renderer{*window};


    return glfwGetWin32Window(window->getGLFWwindow());
}

#ifdef __cplusplus
}
#endif