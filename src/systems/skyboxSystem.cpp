#include "skyboxSystem.hpp"

#include <iostream>
#include <stdexcept>

namespace engine {
    SkyboxSystem::SkyboxSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): device{device} {
        //create Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstant);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }

        //create Pipeline ==================================================

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, device);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.subpass = 1;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        pipelineConfig.colorBlendInfo.pAttachments = &colorBlendAttachment;
        pipelineConfig.colorBlendInfo.attachmentCount = 1;


        pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;

        pipelineConfig.pipelineLayout = pipelineLayout;

        std::vector<std::string> files = {"../../shaders/skybox.vert.spv", "../../shaders/skybox.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<Pipeline>(device, files, flags, pipelineConfig);
    }

    void SkyboxSystem::Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assets, glm::vec4 color) {
        pipeline->bind(commandBuffer);

        PushConstant push{};
        push.color = color;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        
    }
}