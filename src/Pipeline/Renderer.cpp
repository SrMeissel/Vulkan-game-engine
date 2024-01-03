#include "Renderer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>


namespace engine {

    Renderer::Renderer(Window& window, Device& device) : window{window}, device{device} {
        recreateSwapChain();
        createCommandBuffers();
    }
    Renderer::~Renderer() {
        freeCommandBuffers();
    }

    void Renderer::recreateSwapChain() {

        auto extent = window.getExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device.device());

        if(swapchain == nullptr) {
            swapchain = std::make_unique<SwapChain>(device, extent);
        } else {
            std::shared_ptr<SwapChain> oldSwapchain = std::move(swapchain);
            swapchain = std::make_unique<SwapChain>(device, extent, oldSwapchain);
            
            if(!oldSwapchain->compareSwapChain(*swapchain.get())){
                throw std::runtime_error("Swapchain format has changed!");
            }
        }
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if(vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command buffers!");
        }
        
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame() {
        assert(!isFrameStarted && "Cant call begin frame while frame is already is progress!");

        currentRenderPass = 0;

        auto result = swapchain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("Failed to acquire next swapchain image");
        }
        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Command buffer failed to begin recording!");
        }

        return commandBuffer;
    }

    void Renderer::endFrame() {
        assert(isFrameStarted && "Cannot call end frame while no frame is in progress!");
        auto commandBuffer = getCurrentCommandBuffer();

        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
            throw std::runtime_error("Failed to record command buffer!");
        }

        auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()){
            window.resetWindowResizedFlag();
            recreateSwapChain();
        } else if(result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::beginNextRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "cannot call beginNextRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");
        RenderPass renderPass = renderPasses[currentRenderPass];

        VkRenderPassBeginInfo renderPassbeginInfo{};
        renderPassbeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassbeginInfo.renderPass = renderPass.getRenderPass();
        renderPassbeginInfo.framebuffer = renderPass.getFrameBuffer();
        renderPassbeginInfo.renderArea.offset = {0, 0};
        renderPassbeginInfo.renderArea.extent = renderPass.extent;

        std::vector<VkClearValue> clearValues;
        clearValues.resize(renderPass.getRenderPassInfo().attachmentCount);
        for(int i = 0; i < renderPass.getRenderPassInfo().attachmentCount; i++) {
            if(renderPass.getRenderPassInfo().pAttachments[i].format == device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                clearValues[i].depthStencil = {1.0f, 0};
            } else {
                clearValues[i].color = {0.0f, 0.0f, 0.001f, 1.0f};  
            } 
        }

        renderPassbeginInfo.clearValueCount = clearValues.size();
        renderPassbeginInfo.pClearValues = clearValues.data();
        
        vkCmdBeginRenderPass(commandBuffer, &renderPassbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderPass.extent.width);
        viewport.height = static_cast<float>(renderPass.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, renderPass.extent};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::endCurrentRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "cannot call endSwapChainRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);

        currentRenderPass++; // <===========
    }

    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "cannot call beginSwapChainRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassbeginInfo{};
        renderPassbeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassbeginInfo.renderPass = swapchain->getRenderPass();
        renderPassbeginInfo.framebuffer = swapchain->getFrameBuffer(currentImageIndex);
        renderPassbeginInfo.renderArea.offset = {0, 0};
        renderPassbeginInfo.renderArea.extent = swapchain->getSwapChainExtent();

        VkClearValue clearValue;
        clearValue.color = {0.0f, 0.0f, 0.001f, 1.0f};

        renderPassbeginInfo.clearValueCount = 1;
        renderPassbeginInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapchain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    
    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "cannot call endSwapChainRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }
}