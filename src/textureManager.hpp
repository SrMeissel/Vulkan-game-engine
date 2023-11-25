#pragma once

#include "Pipeline/deviceManager.hpp"
#include "bufferManager.hpp" 

#include <../libs/stb/stb_image.h>


#include <memory>

namespace engine{

    struct Texture{
        Texture(Device& device, VkImage image, VkImageView imageView, VkSampler sampler) : device{device}, image{image}, imageView{imageView}, sampler{sampler} {}
        ~Texture();

        VkImageView imageView = nullptr;
        VkImage image = nullptr;
        VkSampler sampler;
        Device& device;
    };  

    class TextureManager{
        public:
            TextureManager(Device& device);

            TextureManager(const TextureManager &) = delete;
            TextureManager& operator=(const TextureManager &) = delete;

            VkSampler getTextureSampler() {return textureSampler; }

            std::unique_ptr<Texture> TextureManager::createTextureFromFile(char * filePath);

        private:

        
            VkImage createTextureImage(char* filePath);
            VkImageView createTextureImageView(VkImage image);

            void createTextureBuffer(VkDeviceSize imageSize, stbi_uc* pixels);
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            void createTextureSampler();

            
            Device &device;
            VkDeviceMemory textureImageMemory;
            VkSampler textureSampler;
    };
}