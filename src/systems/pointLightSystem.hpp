#pragma once

#include "pipeline.hpp"
#include "deviceManager.hpp"
#include "gameObject.hpp"
#include "cameraManager.hpp"
#include "frameInfo.hpp"

#include <memory>
#include <vector>

namespace engine {
    class PointLightSystem {
        public:

            PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~PointLightSystem();

            PointLightSystem(const PointLightSystem &) = delete;
            PointLightSystem &operator=(const PointLightSystem &) = delete;

            void render(frameInfo& frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;
            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;
    };
}