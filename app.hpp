#pragma once

#include "windowManager.hpp"
#include "pipeline.hpp"
#include "deviceManager.hpp"
#include "swapchainManager.hpp"
#include "gameObject.hpp"

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
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);
            void renderGameObjects(VkCommandBuffer commandBuffer);

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device device{window};
            std::unique_ptr<SwapChain> swapchain;
            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<GameObject> gameObjects;
    };
}