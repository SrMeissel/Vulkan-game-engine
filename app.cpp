#include "app.hpp"

namespace engine {
    void app::run() {
        while(!window.shouldClose()){
            glfwPollEvents();
        }
    }
}