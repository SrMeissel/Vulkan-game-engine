#include "Renderer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>


namespace renderer {

    Renderer::Renderer(Window& window) : window{window}, device{window} {

        // Primary renderPass info ================================================================================
        static VkAttachmentDescription attachment;
        attachment.format = SwapChain::chooseSwapSurfaceFormat(device.getSwapChainSupport().formats).format;
        attachment.samples = device.msaaSamples;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // <=========
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.flags = 0;
    
        static VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
        static VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
    
        static VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
        swapchainPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        swapchainPassInfo.attachmentCount = 1;
        swapchainPassInfo.pAttachments = &attachment;
        swapchainPassInfo.subpassCount = 1;
        swapchainPassInfo.pSubpasses = &subpass;
        swapchainPassInfo.dependencyCount = 1;
        swapchainPassInfo.pDependencies = &dependency;

        //========================================================================================================

        recreateSwapChain();
        createCommandBuffers();
        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT).build();

        //init UBO
         for(int i=0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }
        // add UBO to descriptor
        globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

         for(int i=0; i < globalDescriptorSets.size(); i++){
            DescriptorWriter writer(*globalSetLayout, *globalPool);

            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            writer.writeBuffer(0, &bufferInfo);
            writer.build(globalDescriptorSets[i]);
        }

        //create Sampler ==================================================
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &defaultSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

    }
    Renderer::~Renderer() {
        freeCommandBuffers();

        vkDestroySampler(device.device(), defaultSampler, nullptr);
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

    void Renderer::endRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "cannot call endCurrentRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }

    void Renderer::beginRenderPass(VkCommandBuffer commandBuffer, Renderer::DefinedRenderPasses pass) {
        assert(isFrameStarted && "cannot call beginSwapChainRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassbeginInfo{};
        if(pass == Renderer::DefinedRenderPasses::SwapChain){
        renderPassbeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassbeginInfo.renderPass = swapchain->getRenderPass();
        renderPassbeginInfo.framebuffer = swapchain->getFrameBuffer(currentImageIndex);
        renderPassbeginInfo.renderArea.offset = {-1, 0};
        renderPassbeginInfo.renderArea.extent = swapchain->swapChainExtent;
        } else {
            throw std::runtime_error("attempting to start invalid renderpass");
        }

        VkClearValue clearValue;
        clearValue.color = {0.0f, 0.0f, 0.001f, 1.0f};

        renderPassbeginInfo.clearValueCount = 1;
        renderPassbeginInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->swapChainExtent.width);
        viewport.height = static_cast<float>(swapchain->swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapchain->swapChainExtent};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
}