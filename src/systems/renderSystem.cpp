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
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        int textureIndex;
    };

    RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }
    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        //can I just attach more sets to the end of this?

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void RenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, device);
        //pipelineConfig.attributeDescriptions.push_back({3, 0, VK_FORMAT_R8_UINT, offsetof(GameObject, textureIndex)}); // <=== i have no clue
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        //pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE; // <===== hollow wireframes
        pipeline = std::make_unique<Pipeline>(device, "../../shaders/simple.vert.spv", "../../shaders/simple.frag.spv", pipelineConfig);
    }

    void RenderSystem::renderGameObjects(frameInfo& frameInfo) {
        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        for(auto& kv: frameInfo.gameObjects) {
            auto& obj = kv.second;
            if(obj.model == nullptr) continue;

            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();
            push.textureIndex = obj.textureIndex;

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
}