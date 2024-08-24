#include "PointLightSystem.hpp"

#include <stdexcept>

namespace engine {
    PointLightSystem::PointLightSystem(Device& device, RenderPass* renderPass, VkDescriptorSetLayout globalSetLayout) : device(device) {
        //create Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        setLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
        descriptorSetLayouts.push_back(setLayout->getDescriptorSetLayout());

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
        pipelineConfig.renderPass = renderPass->getRenderPass();
        pipelineConfig.subpass = 1;

        pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        pipelineConfig.colorBlendInfo.pAttachments = &colorBlendAttachment;
        pipelineConfig.colorBlendInfo.attachmentCount = 1;

        pipelineConfig.pipelineLayout = pipelineLayout;
        
        std::vector<std::string> files = {"../../shaders/pointlight.vert.spv", "../../shaders/pointlight.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<Pipeline>(device, files, flags, pipelineConfig);

        //create Descriptor Set ==============================================================================

        descriptorPool = engine::DescriptorPool::Builder(device).setMaxSets(3)
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3)
        .build();

        descriptors[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors[0].imageView = renderPass->getAttachmentImageView(0);
        descriptors[0].sampler = VK_NULL_HANDLE;

        descriptors[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors[1].imageView = renderPass->getAttachmentImageView(1);
        descriptors[1].sampler = VK_NULL_HANDLE;

        descriptors[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors[2].imageView = renderPass->getAttachmentImageView(2);
        descriptors[2].sampler = VK_NULL_HANDLE;

        engine::DescriptorWriter writer(*setLayout, *descriptorPool);

        if(writer.writeImages(0, descriptors.data(), 3).build(descriptorSet) == false)
            std::cout << "\n failed to write set \n";
    }

    void PointLightSystem::Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assets) {
        pipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalUBOSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &descriptorSet, 0, nullptr);

        for(auto const& entity : entities) {
            ECS::Transform& transform = assets.GetComponent<ECS::Transform>(entity);
            ECS::PointLight& pointLight = assets.GetComponent<ECS::PointLight>(entity); 

            //push constants ==================================================

            PushConstant push{};
            push.color = glm::vec4(pointLight.color, 1.0);
            push.position = glm::vec4(transform.translation, 1.0);
            push.intensity = pointLight.intensity;
            push.radius = pointLight.radius;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }
    }
}