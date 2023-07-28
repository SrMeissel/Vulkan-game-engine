#include "app.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

namespace engine {

    app::app() {
        loadModels();
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }
    app::~app() {
        vkDestroyPipelineLayout(Device.device(), pipelineLayout, nullptr);
    }

    void app::run() {
        while(!window.shouldClose()){
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(Device.device());
    }

    void app::sierpinski(std::vector<Model::Vertex> &verticies, glm::vec2 left, glm::vec2 right, glm::vec2 top, int iterations) {
        if(iterations <= 0) {
            verticies.push_back({left});
            verticies.push_back({right});
            verticies.push_back({top});
        }
        else {
            auto leftTop = 0.5f*(left + top);
            auto leftRight = 0.5f*(left + right);
            auto rightTop = 0.5f*(right + top);

            sierpinski(verticies, left, leftRight, leftTop, iterations-1);
            sierpinski(verticies, leftRight, right, rightTop, iterations-1);
            sierpinski(verticies, leftTop, rightTop, top, iterations-1);
        }
    }

    void app::loadModels() {
        std::vector<Model::Vertex> verticies {
            //{{0.0f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}
        };
        sierpinski(verticies, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f}, 4);
        model = std::make_unique<Model>(Device, verticies);
    }

    void app::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(Device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void app::createPipeline() {
        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, SwapChain.width(), SwapChain.height());
        pipelineConfig.renderPass = SwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(Device, "../../shaders/simple.vert.spv", "../../shaders/simple.frag.spv", pipelineConfig);
    }

    void app::createCommandBuffers() {
        commandBuffers.resize(SwapChain.imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = Device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if(vkAllocateCommandBuffers(Device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command buffers!");
        }
        for( int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Command buffer failed to begin recording!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = SwapChain.getRenderPass();
            renderPassInfo.framebuffer = SwapChain.getFrameBuffer(i);
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = SwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            pipeline->bind(commandBuffers[i]);
            model->bind(commandBuffers[i]);
            model->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);
            if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS){
                throw std::runtime_error("Failed to record command buffer!");
            }

        }
    }
    void app::drawFrame() {
        uint32_t imageIndex;
        auto result = SwapChain.acquireNextImage(&imageIndex);

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("Failed to acquire next swapchain image");
        }

        result = SwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }
    }
}