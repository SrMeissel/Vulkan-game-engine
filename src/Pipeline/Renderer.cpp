#include "Renderer.hpp"


#include "frameInfo.hpp"

#include <stdexcept>
#include <array>


namespace renderer {

    Renderer::Renderer(Window& window) : window{window}, device{window} {

        // Primary renderPass info ================================================================================

        static std::array<VkAttachmentDescription, 5> attachments;
        static std::array<VkAttachmentReference, 3> colorAttachmentRef = {};
        static std::array<VkAttachmentReference, 3> inputReference = {};

        //colorAttachment
        attachments[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attachments[0].samples = device.msaaSamples;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachmentRef[0].attachment = 0;
        colorAttachmentRef[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        inputReference[0].attachment = 0;
        inputReference[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //normal color attachment
        attachments[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // <================== renderer.device specific
        attachments[1].samples = device.msaaSamples;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachmentRef[1].attachment = 1;
        colorAttachmentRef[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        inputReference[1].attachment = 1;
        inputReference[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //position color attachment
        attachments[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;  // <================== renderer.device specific
        attachments[2].samples = device.msaaSamples;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachmentRef[2].attachment = 2;
        colorAttachmentRef[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        inputReference[2].attachment = 2;
        inputReference[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //depthAttachment
        attachments[3].format = 
	device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
	VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        attachments[3].samples = device.msaaSamples;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        static VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 3;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        //lighting attachment
        attachments[4].format = SwapChain::chooseSwapSurfaceFormat(device.getSwapChainSupport().formats).format;
	attachments[4].samples = device.msaaSamples;
        attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[4].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 

        static VkAttachmentReference lightingAttachmentRef = {};
        lightingAttachmentRef.attachment = 4;
        lightingAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        static std::array<VkSubpassDescription, 2> subpasses {};
            subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[0].colorAttachmentCount = (int)colorAttachmentRef.size();
            subpasses[0].pColorAttachments = colorAttachmentRef.data();
            subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

            subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[1].colorAttachmentCount = 1;
            subpasses[1].pColorAttachments = &lightingAttachmentRef;
            subpasses[1].inputAttachmentCount = (int)inputReference.size();
            subpasses[1].pInputAttachments = inputReference.data();
            subpasses[1].pDepthStencilAttachment = &depthAttachmentRef;

        static std::array<VkSubpassDependency, 1> dependency = {};
            dependency[0].srcSubpass = 0;
            dependency[0].dstSubpass = 1;
            dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Stage of writing to the color attachment
            dependency[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Stage of reading from the attachment in the shader
            dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Access type for writing to the color attachment
            dependency[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; 
            dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        primaryPassInfo = new VkRenderPassCreateInfo{};
        primaryPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        primaryPassInfo->attachmentCount = static_cast<uint32_t>(attachments.size());
        primaryPassInfo->pAttachments = attachments.data();
        primaryPassInfo->subpassCount = (int)subpasses.size();
        primaryPassInfo->pSubpasses = subpasses.data();
        primaryPassInfo->dependencyCount = (int)dependency.size();
        primaryPassInfo->pDependencies = dependency.data();

        primaryRenderPass = std::make_unique<RenderPass>(device, primaryPassInfo, window.getExtent());

        //========================================================================================================

        recreateSwapChain();
        createCommandBuffers();

        globalPool = DescriptorPool::Builder(device)
	.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
	.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT))
	.build();
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

    void Renderer::resizeRenderPasses() {

        auto extent = window.getExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        primaryRenderPass = std::make_unique<RenderPass>(device, primaryPassInfo, extent);
        primaryRenderPass->resized = true;
        std::cout << "aspect ratio: " << primaryRenderPass->getAspectRatio() << "\n";
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

        swapchain->copyImage(commandBuffer, primaryRenderPass->images[4].image, primaryRenderPass->extent, currentImageIndex);

        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
            throw std::runtime_error("Failed to record command buffer!");
        }

        primaryRenderPass->resized=false;
        auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()){
            window.resetWindowResizedFlag();
            recreateSwapChain();
            resizeRenderPasses();
        } else if(result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT; // TODO: figure out why I do this twice
    }

    void Renderer::endRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "cannot call endRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }

    void Renderer::beginRenderPass(VkCommandBuffer commandBuffer, DefinedRenderPasses pass) {
        assert(isFrameStarted && "cannot call beginSwapChainRenderPass if no frame is in progress!");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassbeginInfo{};
        std::vector<VkClearValue> clearValues;

        if(pass == DefinedRenderPasses::Primary){
        renderPassbeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassbeginInfo.renderPass = primaryRenderPass->renderPass;
        renderPassbeginInfo.framebuffer = primaryRenderPass->frameBuffer; 
        renderPassbeginInfo.renderArea.offset = VkOffset2D{0, 0};
        renderPassbeginInfo.renderArea.extent = primaryRenderPass->extent;

        clearValues.resize(primaryPassInfo->attachmentCount);
        for(int i = 0; i < primaryPassInfo->attachmentCount; i++) {
            if(primaryPassInfo->pAttachments[i].format == device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                clearValues[i].depthStencil = {1.0f, 0};
            } else {
                clearValues[i].color = {0.0f, 0.0f, 0.001f, 1.0f};  
            } 
        }
        renderPassbeginInfo.clearValueCount = clearValues.size();
        renderPassbeginInfo.pClearValues = clearValues.data();

        } else {
            throw std::runtime_error("attempting to start invalid renderpass");
        }

        vkCmdBeginRenderPass(commandBuffer, &renderPassbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(primaryRenderPass->extent.width);
        viewport.height = static_cast<float>(primaryRenderPass->extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapchain->swapChainExtent};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
}
