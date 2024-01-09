#include "RenderPass.hpp"

#include <stdexcept>

namespace engine {
    RenderPass::RenderPass(Device& device, Window& window, VkRenderPassCreateInfo* info, bool isWindowExtent, VkExtent2D customExtent) : 
    device{device}, window{window}, info{info}, isWindowExtent{isWindowExtent} {

        if(isWindowExtent) {
            extent = window.getExtent();
        } else {
            extent = customExtent;
        }
        //creates the renderpass objects from info directly
        if (vkCreateRenderPass(device.device(), info, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        createImageResources();
        createFrameBuffer();
    }

    void RenderPass::createImageResources() {
        attachmentMemory.resize(info->attachmentCount);
        attachmentImages.resize(info->attachmentCount);
        attachmentImageViews.resize(info->attachmentCount);

        VkFormat depthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        for(int i = 0; i < info->attachmentCount; i++) {
            //create image =============================================================================================
            VkImageCreateInfo imageInfo{};

            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;

            //does image have a custom extent or draws to window
            if(isWindowExtent) {
                imageInfo.extent.width = window.getExtent().width;
                imageInfo.extent.height = window.getExtent().height;
            } else {
                imageInfo.extent.width = extent.width;
                imageInfo.extent.height = extent.height;
            }

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

            device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachmentImages[i], attachmentMemory[i]);

            //create image view ===========================================================
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = attachmentImages[i];
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

            vkCreateImageView(device.device(), &viewInfo, nullptr, &attachmentImageViews[i]);
        }
    }

    void RenderPass::createFrameBuffer() {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentImageViews.size());
        framebufferInfo.pAttachments = attachmentImageViews.data();

        //does image have a custom extent or draws to window
        if(isWindowExtent) {
            framebufferInfo.width = window.getExtent().width;
            framebufferInfo.height = window.getExtent().height;
        } else {
            framebufferInfo.width = extent.width;
            framebufferInfo.height = extent.height;
        }
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}