#include "Pipeline/windowManager.hpp"
#include "Pipeline/deviceManager.hpp"
#include "Pipeline/Renderer.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/stb/stb_image_write.h"

#include <fstream>
#include <String>
#include <iostream>

namespace engine {
    class ScreenshotTool {
        public:
            
            //assumes blitting is supported
            //and no, I am not execty sure what it does (a boolean based combining or seperating function)
            void ScreenshotTool::takeScreenshot(VkImage srcImage, char* fileName, Device &device, Window window){
                VkImageCreateInfo captureImageInfo;
                VkExtent2D extent = window.getExtent();

                captureImageInfo.imageType = VK_IMAGE_TYPE_2D;
                // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
                captureImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                captureImageInfo.extent.width = extent.width;
                captureImageInfo.extent.height = extent.height;
                captureImageInfo.extent.depth = 1;
                captureImageInfo.arrayLayers = 1;
                captureImageInfo.mipLevels = 1;
                captureImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                captureImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                captureImageInfo.tiling = VK_IMAGE_TILING_LINEAR;
                captureImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

                VkImage cptImage;
                vkCreateImage(device.device(), &captureImageInfo, nullptr, &cptImage);

                //create memory ???
                VkMemoryRequirements memRequirements;
                VkMemoryAllocateInfo memAllocInfo;
                VkDeviceMemory cptImageMemory;
                vkGetImageMemoryRequirements(device.device(), cptImage, &memRequirements);
                memAllocInfo.allocationSize = memRequirements.size;

                memAllocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &cptImageMemory);
                vkBindImageMemory(device.device(), cptImage, cptImageMemory, 0);

                //using the same command buffer

                //changing image layouts 
                device.insertImageMemoryBarrier(
                cptImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
                
                device.insertImageMemoryBarrier(
                srcImage,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                //where blit becomes required, blit can do format conversions
                VkOffset3D blitSize;
                blitSize.x = extent.width;
                blitSize.y = extent.height;
                blitSize.z = 1;
                VkImageBlit imageBlitRegion{};
                imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlitRegion.srcSubresource.layerCount = 1;
                imageBlitRegion.srcOffsets[1] = blitSize;
                imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlitRegion.dstSubresource.layerCount = 1;
                imageBlitRegion.dstOffsets[1] = blitSize;

                VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

                vkCmdBlitImage(
				commandBuffer,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				cptImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);

                device.endSingleTimeCommands(commandBuffer);

                //change image format again to write to file
                device.insertImageMemoryBarrier(
                cptImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                device.insertImageMemoryBarrier(
                srcImage,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                //get image layout
                VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
                VkSubresourceLayout subResourceLayout;
                vkGetImageSubresourceLayout(device.device(), cptImage, &subResource, &subResourceLayout);

                const char* data;
                vkMapMemory(device.device(), cptImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
                data += subResourceLayout.offset;

                // std::ofstream file(fileName, std::ios::out | std::ios::binary);
                // // ppm header
                // file << "P6\n" << extent.width << "\n" << extent.height << "\n" << 255 << "\n";

                // //again assumes color format is correct, slightly want to mess it up just to see BGR in RGB.

                // for(uint32_t y = 0; y < extent.height; y++) {
                //     unsigned int *row = (unsigned int*)data;
                //     for(uint32_t x = 0; x< extent.width; x++) {
                //         file.write((char*)row, 3);
                //     }
                //     data += subResourceLayout.rowPitch;
                // }
                // file.close();
                
                // vkUnmapMemory(device.device(), cptImageMemory);
                // vkFreeMemory(device.device(), cptImageMemory, nullptr);
                // vkDestroyImage(device.device(), cptImage, nullptr);

                // std::cout << "Saved file: " << fileName;

                //using stb to save as jpg not ppm because I can

                //https://stackoverflow.com/questions/56039401/stb-image-write-issue
                stbi_write_jpg(fileName, extent.width, extent.height, 3, (void*)&data, extent.width * sizeof(int));
            }

        private:
        
    };
}