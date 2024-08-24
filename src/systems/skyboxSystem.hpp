#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"

#include <glm/glm.hpp>

namespace engine {
    class SkyboxSystem : public ECS::System {
    public:
        SkyboxSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SkyboxSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

        void SkyboxSystem::cleanup(ECS::AssetSystem& assetManager);

        std::unique_ptr<DescriptorSetLayout>& getSetLayout() { return setLayout; }

    private:
        struct PushConstant {
	        glm::mat4 rotation;
        };

        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;

        std::unique_ptr<DescriptorSetLayout> setLayout;

    };
}