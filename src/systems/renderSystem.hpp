#pragma once

#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include "gameObject.hpp"
#include "cameraManager.hpp"
#include "frameInfo.hpp"

#include <memory>
#include <vector>

namespace engine {
    class RenderSystem {
        public:

            RenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~RenderSystem();

            RenderSystem(const RenderSystem &) = delete;
            RenderSystem &operator=(const RenderSystem &) = delete;

            void renderGameObjects(frameInfo& frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;
            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
    };
}