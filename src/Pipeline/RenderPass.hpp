#pragma once

#include "deviceManager.hpp"
#include "../Utils.hpp"

#include <vector>

namespace engine {
    class RenderPass {
        public:
            RenderPass(Device& device, VkRenderPassCreateInfo* info, VkExtent2D extent = {0,0});
            RenderPass(Device& device, VkRenderPassCreateInfo* info, std::vector<AllocatedImage> images, VkExtent2D extent = {0,0});
            ~RenderPass(); // destroy everything.

            // Not copyable or movable
            RenderPass(const RenderPass &) = delete;
            RenderPass& operator=(const RenderPass &) = delete;
            RenderPass(RenderPass &&) = delete;
            RenderPass& operator=(RenderPass &&) = delete;

            //are images surface resolution? yes/no
            //if so, and window extent changes remake everything.
            
            VkExtent2D extent;
            float getAspectRatio() {return static_cast<float>(extent.width) / static_cast<float>(extent.height); }

            VkRenderPass getRenderPass() {return renderPass; }
            VkRenderPassCreateInfo getRenderPassInfo() {return *info; }
            VkFramebuffer getFrameBuffer() {return frameBuffer; }
            VkImageView getAttachmentImageView(int i) {return images[i].imageView; }

        private:
        // create images
        void createImageResources();
        // create framebuffer
        void createFrameBuffer();

        Device& device;

        VkRenderPassCreateInfo* info;
        VkRenderPass renderPass;
        
        // std::vector<VkDeviceMemory> attachmentMemory; // the data of the image
        // std::vector<VkImage> attachmentImages; // defines format of image
        // std::vector<VkImageView> attachmentImageViews; // which part of image is relevant (Ex. mips)
        //now hear me out.
        std::vector<AllocatedImage> images;

        VkFramebuffer frameBuffer; // connects image to attachment

        //swapchain gets its own custom renderpass not in this class.

    };
}