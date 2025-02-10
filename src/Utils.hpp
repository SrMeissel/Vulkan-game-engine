#pragma once

#include <functional>
#include <iostream>

#define TINYOBJECTLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

namespace engine {
    //whatever teh fuck this is
    
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };
}

    //global structs ==========================================================

namespace renderer {
    struct Vertex{
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec3 tangent{};
        glm::vec3 biTangent{};
        glm::vec2 uv{};

        bool operator ==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    struct AllocatedImage {

        std::string path;

        VkImage image;
        VkImageView imageView;

        VkDeviceMemory memory;
        VkExtent3D imageExtent;
        VkFormat imageFormat;

    };

    // I hope I don't regret this
    struct CubeMap : public AllocatedImage {
        std::string tags; // the base filepath for all images should be the same, but these tags differentiate each face :)
    };

    //vulkan generic functions =====================================================================

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{}; 

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)});
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, biTangent)});
        attributeDescriptions.push_back({5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }



}

//whatever the hell this is =====================================================================

namespace std {
    template<>
    struct hash<renderer::Vertex> {
        size_t operator()(renderer::Vertex const& vertex) const {
            size_t seed = 0;
            engine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed; 
        }
    };
}
