#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"

#include "systems/PointLightSystem.hpp"

#include <glm/glm.hpp>

namespace engine {
    class OmniShadowSystem : public ECS::System {
    public:
        OmniShadowSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~OmniShadowSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager, PointLightSystem& pointLightSystem);

    private:
        struct PushConstant {
            glm::mat4 modelMatrix{1.f};
            glm::mat4 normalMatrix{1.f};
        };

        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
}