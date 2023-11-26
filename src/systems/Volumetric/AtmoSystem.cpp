#include "AtmoSystem.hpp"

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

    AtmoSystem::AtmoSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }
    AtmoSystem::~AtmoSystem() {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void AtmoSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

        // VkPushConstantRange pushConstantRange {};
        // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        // pushConstantRange.offset = 0;
        // pushConstantRange.size = sizeof(SimplePushConstantData);

        // std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        // //can I just attach more sets to the end of this? imma try it
        // //sampler and image set layout only needs to be done once
        // textureSetLayout = DescriptorSetLayout::Builder(device)
        // .addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        // .addBinding(1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
        // .build();   
        // descriptorSetLayouts.push_back(textureSetLayout->getDescriptorSetLayout());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void AtmoSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, device);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        Pipeline::enableAlphaBlending(pipelineConfig);

        //pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE; // <===== hollow wireframes

        std::vector<std::string> files = { "../../shaders/Volumetrics/FullscreenQuad.vert.spv", "../../shaders/Volumetrics/RayMarcher.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<Pipeline>(device, files, flags, pipelineConfig);
    }

    void AtmoSystem::renderGameObjects(frameInfo& frameInfo) {
        pipeline->bind(frameInfo.commandBuffer);

        vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);

        // //vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        // for(auto& kv: frameInfo.gameObjects) {

        //     auto& obj = kv.second;
        //     if(obj.model == nullptr)
        //     continue;

        //     if(obj.texture == nullptr)
        //     continue;

        //     // SimplePushConstantData push{};
        //     // push.modelMatrix = obj.transform.mat4();
        //     // push.normalMatrix = obj.transform.normalMatrix();
        //     // push.textureIndex = obj.textureIndex;   

        //     //vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        //     obj.model->bind(frameInfo.commandBuffer);
        //     obj.model->draw(frameInfo.commandBuffer);
        // }
        //std::cout << "finished rendering objects" << "\n";
    }
}