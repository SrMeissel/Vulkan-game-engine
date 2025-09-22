#pragma once

#include "ECS/AssetManager.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"

#include "systems/PointLightSystem.hpp"

#include <glm/glm.hpp>

namespace engine {
    class OmniShadowSystem {
    public:
        OmniShadowSystem(renderer::Device& device, ECS::AssetSystem& assetManager, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~OmniShadowSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager, PointLightSystem& pointLightSystem);

    private:
        struct PushConstant {
            glm::mat4 modelMatrix{1.f};
            glm::mat4 normalMatrix{1.f};
        };

        renderer::Device &device;

        std::unique_ptr<renderer::Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<ECS::System> entities;
    };
}