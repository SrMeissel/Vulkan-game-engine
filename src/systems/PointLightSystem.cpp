#include "PointLightSystem.hpp"
#include <stdexcept>

namespace engine {
    PointLightSystem::PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device(device) {
        //create Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstant);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
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

        pipelineConfig.pipelineLayout = pipelineLayout;
        
        std::vector<std::string> files = {"../../shaders/mesh.vert.spv", "../../shaders/mesh.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<Pipeline>(device, files, flags, pipelineConfig);
    }

    void PointLightSystem::Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assets) {
        pipeline->bind(commandBuffer);

        for(auto const& entity : entities) {
            ECS::Transform& transform = assets.GetComponent<ECS::Transform>(entity);
            ECS::PointLight& pointLight = assets.GetComponent<ECS::PointLight>(entity); 

            //push constants ==================================================

            PushConstant push{};
            push.color = pointLight.color;
            push.position = transform.translation;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);


            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }
    }
}