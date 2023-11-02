#include "screenshotTool.hpp"

//#include "Pipeline/windowManager.hpp"
//#include "Pipeline/deviceManager.hpp"
//#include "Pipeline/Renderer.hpp"

 #define STB_IMAGE_WRITE_STATIC
 #define STB_IMAGE_WRITE_IMPLEMENTATION
 #include "../libs/stb/stb_image_write.h"

 #include <fstream>
// #include <String>
 #include <iostream>

namespace engine {
            //assumes blitting is supported
            //Turns out that was a bad idea my 1060 does not in fact support blitting https://vulkan.gpuinfo.org/displayreport.php?id=25309#formats
            //this is only for linear tiling, it is supprted for optimal tiling but i dont want to dive into something new right now
            //and no, I am not execty sure what it does (a boolean based combining or seperating function)
            
            void ScreenshotTool::takeScreenshot(VkImage srcImage, char* fileName, Device &device, VkExtent2D extent){
                bool supportsblit = false;
                //check for blit support later

                VkImageCreateInfo captureImageInfo{};

                captureImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                captureImageInfo.imageType = VK_IMAGE_TYPE_2D;
                // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
                captureImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // specifies a four-component, 32-bit unsigned normalized format that has an 8-bit R component in byte 0, an 8-bit G component in byte 1, an 8-bit B component in byte 2, and an 8-bit A component in byte 3.
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

                //create memory
                VkMemoryRequirements memRequirements;
                VkMemoryAllocateInfo memAllocInfo{};
                VkDeviceMemory cptImageMemory;
                vkGetImageMemoryRequirements(device.device(), cptImage, &memRequirements);
                memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                memAllocInfo.allocationSize = memRequirements.size;

                memAllocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &cptImageMemory);
                vkBindImageMemory(device.device(), cptImage, cptImageMemory, 0);

                VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

                //changing image layouts back to original
                device.insertImageMemoryBarrier(
                commandBuffer,
                cptImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
                
                device.insertImageMemoryBarrier(
                commandBuffer,
                srcImage,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                //where blit becomes required, blit can do format conversions
                if(supportsblit){
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

                    vkCmdBlitImage(
                    commandBuffer,
                    srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    cptImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageBlitRegion,
                    VK_FILTER_NEAREST);
                }
                //will need to manually convert to rgb, eventually
                if(!supportsblit) {
                    VkImageCopy imageCopyRegion{};
                    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageCopyRegion.srcSubresource.layerCount = 1;
                    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageCopyRegion.dstSubresource.layerCount = 1;
                    imageCopyRegion.extent.width = extent.width;
                    imageCopyRegion.extent.height = extent.height;
                    imageCopyRegion.extent.depth = 1;

                    //Issue the copy command
                    vkCmdCopyImage(
                        commandBuffer,
                        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        cptImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &imageCopyRegion);
                }

                //change image format again to write to file
                device.insertImageMemoryBarrier(
                commandBuffer,
                cptImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                device.insertImageMemoryBarrier(
                commandBuffer,
                srcImage,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                device.endSingleTimeCommands(commandBuffer);

                //get image layout
                VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
                VkSubresourceLayout subResourceLayout;
                vkGetImageSubresourceLayout(device.device(), cptImage, &subResource, &subResourceLayout);

                

                const char* data;
                vkMapMemory(device.device(), cptImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);

                //using stb to save as jpg not ppm because I can    
                // technically, I *could* go into the write_jpg function and replace dataR with dataB...

                //https://stackoverflow.com/questions/56039401/stb-image-write-issue


                if(stbi_write_jpg(fileName, extent.width, extent.height, 4, (void*)data, 100) != 0) {
                    std::cout << "Image successfully saved! \n";
                }

                vkUnmapMemory(device.device(), cptImageMemory);
		        vkFreeMemory(device.device(), cptImageMemory, nullptr);
		        vkDestroyImage(device.device(), cptImage, nullptr);
    };
}