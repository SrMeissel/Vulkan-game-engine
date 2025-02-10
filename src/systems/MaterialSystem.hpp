#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include <descriptorManager.hpp>

#include <glm/glm.hpp>


namespace engine {
    class MaterialSystem : public ECS::System {
    public:
        MaterialSystem(renderer::Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~MaterialSystem() { 
            vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
            vkDestroySampler(device.device(), sampler, nullptr);
        }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

        void cleanup(ECS::AssetSystem& assetManager);

        VkSampler& getSampler() { return sampler; }
        std::unique_ptr<renderer::DescriptorSetLayout>& getMaterialSetLayout() { return materialSetLayout; }

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
    };
}