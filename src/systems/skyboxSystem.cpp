#include "skyboxSystem.hpp"
#include "ECS/AssetManager.hpp"

#include <stdexcept>

// https://www.youtube.com/watch?v=iRepdTM8oqI
// this was helpful

// https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330?newreg=ea7231270e514471ac6dcc7370d80bef
// this was awesome

namespace engine {
    SkyboxSystem::SkyboxSystem(renderer::Device& device, ECS::AssetSystem& assetManager, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): device{device} {
        
        entities = assetManager.RegisterSystem(systemName);
       
        ECS::Signature skyboxSigniture;
        skyboxSigniture.set(assetManager.GetComponentType<ECS::Transform>());
        skyboxSigniture.set(assetManager.GetComponentType<ECS::SkyBox>());
        assetManager.SetSystemSignature(skyboxSigniture, systemName);

        //create Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        setLayout = renderer::DescriptorSetLayout::Builder(device).addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();
        descriptorSetLayouts.push_back(setLayout->getDescriptorSetLayout());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }

        //create Pipeline ==================================================

        renderer::PipelineConfigInfo pipelineConfig{};
        renderer::Pipeline::defaultPipelineConfigInfo(pipelineConfig, device);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.subpass = 1;

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


        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE; // Disable depth writes
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        pipelineConfig.pipelineLayout = pipelineLayout;

        std::vector<std::string> files = {(std::string)SOURCE_PATH + "/shaders/skybox.vert.spv", (std::string)SOURCE_PATH + "/shaders/skybox.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<renderer::Pipeline>(device, files, flags, pipelineConfig);

        //make pipeline =================================================================================
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
        
        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void SkyboxSystem::Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assets) {
        assert(entities->size() <= 1 && "Why are there more than one skybox?");
        //maybe I could make multiple skymaps add to each other, but why?

        for(auto& entity : *entities) {

            pipeline->bind(commandBuffer);
            
            ECS::SkyBox& skybox = assets.GetComponent<ECS::SkyBox>(entity);
            ECS::Transform& transform = assets.GetComponent<ECS::Transform>(entity);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalUBOSet, 0, nullptr);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &skybox.descriptorSet, 0, nullptr);

            glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 rotationMatrix = rotationZ * rotationY * rotationX; // maybe reverse, idk man
            
            PushConstant push{};
            push.rotation = rotationMatrix;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }
        
    }
}
