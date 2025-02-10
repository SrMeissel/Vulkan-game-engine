#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"

#include <glm/glm.hpp>

namespace engine {
    class SkyboxSystem : public ECS::System {
    public:
        SkyboxSystem(renderer::Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SkyboxSystem() {
            vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
            vkDestroySampler(device.device(), sampler, nullptr);
        }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

        void cleanup(ECS::AssetSystem& assetManager);

        VkSampler& getSampler() { return sampler; }
        std::unique_ptr<renderer::DescriptorSetLayout>& getSetLayout() { return setLayout; }

    private:
        struct PushConstant {
	        glm::mat4 rotation;
        };

        renderer::Device &device;

        std::unique_ptr<renderer::Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
         
         VkSampler sampler;

        std::unique_ptr<renderer::DescriptorSetLayout> setLayout;

    };
}