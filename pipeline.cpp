#include "pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace engine {
    
    Pipeline::Pipeline(engine::Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const pipelineConfigInfo& configInfo) : Device{device} {
        createGraphicspipeline(vertFilepath, fragFilepath, configInfo);
    }

    std::vector<char> Pipeline::readFile(const std::string& filePath) {
        std::ifstream file{filePath, std::ios::ate | std::ios::binary};

        if(!file.is_open()){
            throw std::runtime_error("failed to open file: " + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void Pipeline::createGraphicspipeline(const std::string& vertFilePath, const std::string& fragFilepath, const pipelineConfigInfo& configInfo) {
        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilepath);

        std::cout << "Vertex Shader Code Size: " << vertCode.size() << "\n";
        std::cout << "fragment Shader Code Size: " << fragCode.size() << "\n";
    }

    void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule){
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(Device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module");
        }
    }

    pipelineConfigInfo Pipeline::defaultPipelineConifInfo(uint32_t width, uint32_t height) {
        pipelineConfigInfo configInfo{};

        return configInfo;
    }
}