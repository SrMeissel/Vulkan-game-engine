#pragma once

#include "deviceManager.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace engine {
    class Model {
        public:

            struct Vertex{
                glm::vec2 position;
                glm::vec3 color;

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            Model(Device &device, const std::vector<Vertex> &verticies);
            ~Model();

            Model(const Model &) = delete;
            Model& operator=(const Model &) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);
            
        private:
            void createVertexBuffers(const std::vector<Vertex> &verticies);


            Device &device;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            uint32_t vertexCount;

    };
}