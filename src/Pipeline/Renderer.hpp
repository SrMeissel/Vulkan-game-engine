#pragma once

#include "windowManager.hpp" 
#include "deviceManager.hpp"
#include "swapchainManager.hpp"
#include "RenderPass.hpp" // <===========

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

            float getAspectRatio() const {return swapchain->extentAspectRatio(); }
            std::vector<VkImage> getSwapchainImages() const {return swapchain->getImages(); }
            VkRenderPass getSwapchainRenderPass() {return swapchain->getRenderPass(); } // <=============
            bool isFrameInProgress() const { return isFrameStarted; }

            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get frame buffer when frame is in progress!");
                return commandBuffers[currentFrameIndex];    
            }

            // FRAME INDEX IS DIFFERENT THAN IMAGE INDEX !!!!!
            int getFrameIndex() const {
                assert(isFrameStarted && "Cannot get frame index while frame is not in progress!");
                return currentFrameIndex;
            }
            uint32_t getCurrentImageIndex() const {return currentImageIndex; }

            void appendRenderPass(RenderPass renderPass) {renderPasses.push_back(renderPass); }
            RenderPass getRenderPass(int i) {return renderPasses[i]; }

            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void beginNextRenderPass(VkCommandBuffer commandBuffer);
            void endCurrentRenderPass(VkCommandBuffer commandBuffer);

        private:
            void createCommandBuffers();
            void freeCommandBuffers();

            void recreateSwapChain();
            void ResizeRenderPasses(); // if renderpass uses window extent that needs to be resizes with the window

            Window& window;
            Device& device;

            std::unique_ptr<SwapChain> swapchain;
            std::vector<VkCommandBuffer> commandBuffers;

            std::vector<RenderPass> renderPasses;
            int currentRenderPass{0};

            // Image index and frame index are different things!!!!!
            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted = false;
    };
}