#include "MaterialSystem.hpp"
#include "ECS/Components.hpp"

#include <stdexcept>

namespace engine {
    MaterialSystem::MaterialSystem(renderer::Device& device, ECS::AssetSystem& assetManager, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): device{device} {

        entities = assetManager.RegisterSystem(systemName);

        ECS::Signature materialSignature;
        materialSignature.set(assetManager.GetComponentType<ECS::Transform>());
        materialSignature.set(assetManager.GetComponentType<ECS::Renderable>());
        materialSignature.set(assetManager.GetComponentType<ECS::Material>());
        assetManager.SetSystemSignature(materialSignature, systemName);

        //create Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        materialSetLayout = renderer::DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
        descriptorSetLayouts.push_back(materialSetLayout->getDescriptorSetLayout());

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

        renderer::PipelineConfigInfo pipelineConfig{};
        renderer::Pipeline::defaultPipelineConfigInfo(pipelineConfig, device);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.subpass = 0;

        pipelineConfig.pipelineLayout = pipelineLayout;
        
        std::vector<std::string> files = {(std::string)SOURCE_PATH + "/shaders/material.vert.spv", (std::string)SOURCE_PATH + "/shaders/material.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        pipeline = std::make_unique<renderer::Pipeline>(device, files, flags, pipelineConfig);

        //create Sampler ==================================================
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

    void MaterialSystem::Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager) {
        pipeline->bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalUBOSet, 0, nullptr);

        for(auto const& entity : *entities) {    

            //get components ==================================================
    
            ECS::Transform& transform = assetManager.GetComponent<ECS::Transform>(entity);
            ECS::Renderable& mesh = assetManager.GetComponent<ECS::Renderable>(entity);
            ECS::Material& material = assetManager.GetComponent<ECS::Material>(entity);
    
            //push constants ==================================================
            PushConstant push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);

            //bind material descriptor sets ==================================================

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &material.descriptorSet, 0, nullptr);

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
    }
}
