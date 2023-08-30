#pragma once

#include "windowManager.hpp"
#include "deviceManager.hpp"
#include "gameObject.hpp"
#include "Renderer.hpp"

#include <memory>
#include <vector>

namespace engine {
    class app {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            app();
            ~app();

            app(const app &) = delete;
            app &operator=(const app &) = delete;

            void run();
        private:
            void loadGameObjects();

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device device{window};
            Renderer renderer{window, device};
            std::vector<GameObject> gameObjects;

    };
}