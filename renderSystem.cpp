#include "renderSystem.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {

    struct SimplePushConstantData {
        glm::mat4 transform{1.f};
        glm::mat4 modelMatrix{1.f};
    };

    RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass) : device{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }
    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout() {

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void RenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(device, "../../shaders/simple.vert.spv", "../../shaders/simple.frag.spv", pipelineConfig);
    }

    void RenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects, const CameraManager& camera) {
        pipeline->bind(commandBuffer);

        auto projectionView = camera.getProjection() * camera.getView();

        for(auto& obj: gameObjects) {

            SimplePushConstantData push{};
            auto modelMatrix = obj.transform.mat4();
            push.transform = projectionView * modelMatrix;
            push.modelMatrix = modelMatrix;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}