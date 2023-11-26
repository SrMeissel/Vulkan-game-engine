#pragma once

#include "deviceManager.hpp"

#include <string>
#include <vector>

namespace engine{

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
        public:
            Pipeline(Device& device, std::vector<std::string> files, std::vector<VkShaderStageFlagBits> flags, const PipelineConfigInfo& configInfo);
            ~Pipeline();

            Pipeline() = default;
            Pipeline(const Pipeline&) = delete;
            Pipeline& operator=(const Pipeline&) = delete;

            static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, Device& device);
            static void enableAlphaBlending(PipelineConfigInfo& configInfo);

            void bind(VkCommandBuffer commandBuffer);

        private:
            static std::vector<char> readFile(const std::string& filePath);

            void createGraphicsPipeline(std::vector<std::string> files, std::vector<VkShaderStageFlagBits> flags, const PipelineConfigInfo& configInfo); 

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

            Device& Device;
            VkPipeline graphicsPipeline;
            std::vector<VkShaderModule> modules{};
            //VkShaderModule vertShaderModule;
            //VkShaderModule fragShaderModule;
    };
}