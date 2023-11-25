#pragma once

#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include "gameObject.hpp"
#include "cameraManager.hpp"
#include "frameInfo.hpp"
#include "descriptorManager.hpp"
#include "Components/textureManager.hpp"

#include <memory>
#include <vector>

namespace engine {
    class AtmoSystem {
        public:

            AtmoSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~AtmoSystem();

            AtmoSystem(const AtmoSystem &) = delete;
            AtmoSystem &operator=(const AtmoSystem &) = delete;

            void renderGameObjects(frameInfo& frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            Device &device;
            std::unique_ptr<Pipeline> pipeline;
            VkPipelineLayout pipelineLayout;

            std::unique_ptr<DescriptorSetLayout> textureSetLayout;
            std::shared_ptr<DescriptorPool> pool;
    };
}