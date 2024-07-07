#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include <descriptorManager.hpp>

#include <glm/glm.hpp>

namespace engine {
    class PointLightSystem : public ECS::System {
    public:
        PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
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
    };
}