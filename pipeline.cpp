#include "pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace engine {
    
    Pipeline::Pipeline(const std::string& vertFilepath, const std::string& fragFilepath) {
        createGraphicspipline(vertFilepath, fragFilepath);
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

    void Pipeline::createGraphicspipline(const std::string& vertFilePath, const std::string& fragFilepath) {
        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilepath);

        std::cout << "Vertex Shader Code Size: " << vertCode.size() << "\n";
        std::cout << "fragment Shader Code Size: " << fragCode.size() << "\n";
    }
}