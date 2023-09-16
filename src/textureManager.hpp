#pragma once

#include "Pipeline/deviceManager.hpp"
#include "bufferManager.hpp" 

#include <../libs/stb/stb_image.h>


#include <memory>

namespace engine{

    class Texture{
        public:
            Texture(Device& device, char* filePath);
            ~Texture();

            Texture(const Texture &) = delete;
            Texture& operator=(const Texture &) = delete;


            VkImageView getTextureImageView() {return textureImageView; }
            VkSampler getTextureSampler() {return textureSampler; }

        private:
            std::unique_ptr<Texture> createTextureFromFile(Device& device, char filePath);

            void createTextureImage(char* filePath);
            void createTextureBuffer(VkDeviceSize imageSize, stbi_uc* pixels);
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            void createTextureImageView();
            void createTextureSampler();

            
            Device &device;
            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            VkImageView textureImageView;
            VkSampler textureSampler;
    };
}