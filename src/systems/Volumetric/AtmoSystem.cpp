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


        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        inputSetLayout = DescriptorSetLayout::Builder(device).addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT).build();
        //VkDescriptorSetLayout layout = inputSetLayout->getDescriptorSetLayout();

        descriptorSetLayouts.push_back(inputSetLayout->getDescriptorSetLayout());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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
        Pipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.subpass = 1;
        
        pipelineConfig.pipelineLayout = pipelineLayout;

        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        std::vector<std::string> files = { "../../shaders/Volumetrics/FullscreenQuad.vert.spv", "../../shaders/Volumetrics/RayMarcher.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<Pipeline>(device, files, flags, pipelineConfig);
    }

    void AtmoSystem::renderAtmosphere(frameInfo& frameInfo, VkImageView& depthImageView) {
        pipeline->bind(frameInfo.commandBuffer);

        createMemoryObjects(frameInfo, depthImageView);

        vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);

        //std::cout << "finished rendering sky" << "\n";
    }

    void AtmoSystem::createMemoryObjects(frameInfo& frameInfo, VkImageView& depthImageView) {
        //bind depth descriptor stuff, same process as simple renderer
        std::shared_ptr<DescriptorPool> texturePool = DescriptorPool::Builder(device).setMaxSets(1)
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1)
        .build();

        VkDescriptorImageInfo imageInfo;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = depthImageView;
        imageInfo.sampler = VK_NULL_HANDLE;

        VkDescriptorSet descriptorSet;
        DescriptorWriter writer(*inputSetLayout, *texturePool);
        if(writer.writeImage(0, &imageInfo, 1).build(descriptorSet) != true){
            std::cout << "\n this thing failed \n";
        }

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    }
}