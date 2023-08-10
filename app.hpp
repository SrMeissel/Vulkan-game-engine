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
            void sierpinski(std::vector<Model::Vertex> &verticies, glm::vec2 left, glm::vec2 right, glm::vec2 top, int iterations);
            void loadGameObjects();

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device device{window};
            Renderer renderer{window, device};
            std::vector<GameObject> gameObjects;

    };
}