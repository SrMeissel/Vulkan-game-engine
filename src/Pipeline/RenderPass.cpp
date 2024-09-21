#include "RenderPass.hpp"

#include <stdexcept>
#include <iostream>

namespace engine {
    RenderPass::RenderPass(Device& device, VkRenderPassCreateInfo* info, VkExtent2D customExtent) : 
    device{device}, info{info}, extent{customExtent} {
        //creates the renderpass objects from info directly
        if (vkCreateRenderPass(device.device(), info, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        createImageResources();
        createFrameBuffer();
    }

    RenderPass::RenderPass(Device& device, VkRenderPassCreateInfo* info, std::vector<AllocatedImage> images, VkExtent2D customExtent) :
    device{device}, info{info}, images{images}, extent{customExtent} {
       
        if (vkCreateRenderPass(device.device(), info, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        createFrameBuffer();
    }

    RenderPass::~RenderPass() {
        for (size_t i = 0; i < images.size(); i++) {
            vkDestroyImageView(device.device(), images[i].imageView, nullptr);
            vkDestroyImage(device.device(), images[i].image, nullptr);
            vkFreeMemory(device.device(), images[i].memory, nullptr);
        }
        vkDestroyFramebuffer(device.device(), frameBuffer, nullptr);
        vkDestroyRenderPass(device.device(), renderPass, nullptr);
    }
    //copilot wrote this, it actually looks good.

    void RenderPass::createImageResources() {
        images.resize(info->attachmentCount);

        VkFormat depthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        for(int i = 0; i < info->attachmentCount; i++) {
            //create image =============================================================================================
            VkImageCreateInfo imageInfo{};

            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;

            //does image have a custom extent or draws to window
            imageInfo.extent.width = extent.width;
            imageInfo.extent.height = extent.height;

            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = info->pAttachments[i].format; // <========
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            //checks if image is depth or color
            if(info->pAttachments[i].format == depthFormat) {
                imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            }
            else {
                imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // <===========  VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
            }

            imageInfo.samples = device.msaaSamples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images[i].image, images[i].memory);

            //create image view ===========================================================
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = images[i].image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = info->pAttachments[i].format;

            //checks if image is depth or color
            if(info->pAttachments[i].format == depthFormat) {
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            else {
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(device.device(), &viewInfo, nullptr, &images[i].imageView);
        }
    }

    void RenderPass::createFrameBuffer() {

        std::vector<VkImageView> imageViews;
        for(int i = 0; i < images.size(); i ++) {
            imageViews.push_back(images[i].imageView);
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(images.size());
        framebufferInfo.pAttachments = imageViews.data();

        //does image have a custom extent or draws to window
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}