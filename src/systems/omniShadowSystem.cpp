#include "omniShadowSystem.hpp"

#include <iostream>
#include <stdexcept>

namespace engine {
    OmniShadowSystem::OmniShadowSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): device{device} {
        //create Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }

        //create Pipeline ==================================================

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, device);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.subpass = 0;

        pipelineConfig.pipelineLayout = pipelineLayout;
        //pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE; // <===== hollow wireframes
        
        std::vector<std::string> files = {"../../shaders/mesh.vert.spv", "../../shaders/mesh.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<Pipeline>(device, files, flags, pipelineConfig);
    }

    void OmniShadowSystem::Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assets, PointLightSystem& pointLightSystem) {
        for(auto& pointlightEntity : pointLightSystem.entities ) {
            ECS::PointLight& pointLight = assets.GetComponent<ECS::PointLight>(pointlightEntity);

            //begin renderpass ==================================================

            

            //fill shadowmap ==================================================

            //pipeline->bind(commandBuffer);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalUBOSet, 0, nullptr);

            for(auto const& entity : entities) {

                //get components ==================================================

                ECS::Transform& transform = assets.GetComponent<ECS::Transform>(entity);
                ECS::Renderable& mesh = assets.GetComponent<ECS::Renderable>(entity);

                //push constants ==================================================

                PushConstant push{};
                push.modelMatrix = transform.mat4();
                push.normalMatrix = transform.normalMatrix();
                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);

                //bind vertex buffer ==================================================

                VkBuffer buffers[] = {mesh.vertexBuffer->getBuffer()};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
                if(mesh.hasIndexBuffer) {
                    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
                }

                //draw ==================================================

                if(mesh.hasIndexBuffer) {
                    vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
                } else{
                    vkCmdDraw(commandBuffer, mesh.vertexCount, 1, 0, 0);
                }
            }

            //end renderpass ==================================================

        }
        
    }
}