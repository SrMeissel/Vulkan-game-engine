#pragma once

#include "ECS/AssetManager.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"

#include <glm/glm.hpp>

namespace engine {
    class MeshSystem {
    public:
        MeshSystem(renderer::Device& device, ECS::AssetSystem& assetManager, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~MeshSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }

        void Render(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManager);

        const std::string systemName{"Mesh"};
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