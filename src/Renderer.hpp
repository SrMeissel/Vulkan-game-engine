#pragma once

#include "windowManager.hpp" 
#include "deviceManager.hpp"
#include "swapchainManager.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace engine {
    class Renderer {
        public:
            Renderer(Window& window, Device& device);
            ~Renderer();

            Renderer(const Renderer &) = delete;
            Renderer &operator=(const Renderer &) = delete;

            VkRenderPass getRenderPass() const { return swapchain->getRenderPass();}
            float getAspectRatio() const {return swapchain->extentAspectRatio(); }
            bool isFrameInProgress() const { return isFrameStarted; }


            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get frame buffer when frame is in progress!");
                return commandBuffers[currentFrameIndex];    
            }

            int getFrameIndex() const {
                assert(isFrameStarted && "Cannot get frame index while frame is not in progress!");
                return currentFrameIndex;
            }

            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

            Window& window;
            Device& device;
            std::unique_ptr<SwapChain> swapchain;
            std::vector<VkCommandBuffer> commandBuffers;

            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted = false;
    };
}