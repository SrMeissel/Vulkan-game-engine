#pragma once

#include "deviceManager.hpp"

#include <string>
#include <vector>

namespace engine{

    struct pipelineConfigInfo {

    };

    class Pipeline {
        public:
            Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const pipelineConfigInfo& configInfo);
            ~Pipeline() {}

            Pipeline(const Pipeline&) = delete;
            void operator=(const Pipeline&) = delete;

            static pipelineConfigInfo defaultPipelineConifInfo(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string& filePath);

            void createGraphicspipeline(const std::string& vertFilepath, const std::string& fragFilepath, const pipelineConfigInfo& configInfo); 

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

            Device& Device;
            VkPipeline graphicsPipeline;
            VkShaderModule vertShaderModule;
            VkShaderModule fragShaderModule;
    };
}