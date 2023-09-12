#pragma once

#include "Pipeline/deviceManager.hpp"
#include "bufferManager.hpp" 

#include <memory>

namespace engine{
    class Texture{
        public:
            Texture(Device& device);
            ~Texture();

            Texture(const Texture &) = delete;
            Texture& operator=(const Texture &) = delete;

            void createTextureImage(Device& device);

        private:
            void createTextureBuffer(VkDeviceSize imageSize, stbi_uc* pixels);
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

            Device &device;
            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
    };
}