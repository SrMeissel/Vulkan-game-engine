#pragma once

#include "Pipeline/deviceManager.hpp"
#include "bufferManager.hpp" 

#include <../libs/stb/stb_image.h>


#include <memory>

namespace engine{
    struct Texture{
        VkImageView imageView = nullptr;
        VkImage image = nullptr;
        //Device& device;

    //Texture(Device &device);
    //~Texture();
    };  

    class TextureManager{
        public:
            TextureManager(Device& device);
            ~TextureManager();

            TextureManager(const TextureManager &) = delete;
            TextureManager& operator=(const TextureManager &) = delete;

            VkSampler getTextureSampler() {return textureSampler; }

            VkImage createTextureImage(char* filePath);
            VkImageView createTextureImageView(VkImage image);
            void destroyTexture(Texture texture);

        private:
            std::unique_ptr<TextureManager> createTextureFromFile(Device& device, char filePath);

            void createTextureBuffer(VkDeviceSize imageSize, stbi_uc* pixels);
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            void createTextureSampler();

            
            Device &device;
            VkDeviceMemory textureImageMemory;
            VkSampler textureSampler;
    };
}