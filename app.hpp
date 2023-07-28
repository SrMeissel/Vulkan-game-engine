#pragma once

#include "windowManager.hpp"
#include "pipeline.hpp"
#include "deviceManager.hpp"
#include "swapchainManager.hpp"
#include "modelManager.hpp"

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
            void loadModels();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device Device{window};
            SwapChain SwapChain{Device, window.getExtent()};
            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::unique_ptr<Model> model;
    };
}