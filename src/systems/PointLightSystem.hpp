#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include "Pipeline/RenderPass.hpp"
#include "Pipeline/Renderer.hpp"
#include <descriptorManager.hpp>

#include <glm/glm.hpp>

// http://www.cemyuksel.com/research/pointlightattenuation/

namespace engine {
    class PointLightSystem : public ECS::System {
    public:
        PointLightSystem(renderer::Renderer& renderer);
        ~PointLightSystem() { 
            vkDestroyPipelineLayout(renderer.device.device(), pipelineLayout, nullptr); 
        }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

    private:
        struct PushConstant {
            glm::vec4 position;
            glm::vec4 color;
            float intensity;
            float radius;
        };

        void recreateDescriptorSets();

        renderer::Renderer& renderer;

        std::unique_ptr<renderer::Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<renderer::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;
        std::unique_ptr<renderer::DescriptorSetLayout> setLayout;
        std::array<VkDescriptorSetLayout, 1> setLayoutData{};
        std::array<VkDescriptorImageInfo, 3> descriptors{};
    };
}