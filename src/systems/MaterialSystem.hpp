#pragma once

#include "ECS/AssetManager.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include "descriptorManager.hpp"

#include <glm/glm.hpp>


namespace engine {
    class MaterialSystem {
    public:
        MaterialSystem(renderer::Device& device, ECS::AssetSystem& assetManager, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~MaterialSystem() { 
            vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
            vkDestroySampler(device.device(), sampler, nullptr);
        }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

        VkSampler& getSampler() { return sampler; }
        std::unique_ptr<renderer::DescriptorSetLayout>& getMaterialSetLayout() { return materialSetLayout; }

        const std::string systemName{"Material"};

    private:
        struct PushConstant {
            glm::mat4 modelMatrix{1.f};
            glm::mat4 normalMatrix{1.f};
        };

        renderer::Device &device;

        std::unique_ptr<renderer::Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        VkSampler sampler;
        std::unique_ptr<renderer::DescriptorSetLayout> materialSetLayout;

        std::shared_ptr<ECS::System> entities;
    };
}