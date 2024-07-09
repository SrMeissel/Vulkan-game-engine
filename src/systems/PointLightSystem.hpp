#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include "../Pipeline/RenderPass.hpp"
#include <descriptorManager.hpp>

#include <glm/glm.hpp>

namespace engine {
    class PointLightSystem : public ECS::System {
    public:
        PointLightSystem(Device& device, RenderPass* renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

    private:
        struct PushConstant {
            glm::vec3 position;
            glm::vec3 color;
        };

        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;

        VkSampler sampler;
        std::shared_ptr<engine::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;
        std::unique_ptr<DescriptorSetLayout> setLayout;
        std::array<VkDescriptorSetLayout, 1> setLayoutData{};
        std::array<VkDescriptorImageInfo, 3> descriptors{};
    };
}