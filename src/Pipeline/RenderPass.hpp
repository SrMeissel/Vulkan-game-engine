#pragma once

#include "deviceManager.hpp"

#include <vector>

namespace engine {
    class RenderPass {
        public:
            RenderPass(Device& device, Window& window, VkRenderPassCreateInfo* info, bool isWindowExtent, VkExtent2D extent = {0,0});
            ~RenderPass() {}; // destroy everything.

            //are images surface resolution? yes/no
            //if so, and window extent changes remake everything.
            bool isWindowExtent;
            VkExtent2D extent;

            VkRenderPass getRenderPass() {return renderPass; }
            VkRenderPassCreateInfo getRenderPassInfo() {return *info; }
            VkFramebuffer getFrameBuffer() {return frameBuffer; }
            VkImageView getAttachmentImageView(int i) {return attachmentImageViews[i]; }

        private:
        // create images
        void createImageResources();
        // create framebuffer
        void createFrameBuffer();

        Device& device;
        Window& window;

        VkRenderPassCreateInfo* info;
        VkRenderPass renderPass;
        
        std::vector<VkDeviceMemory> attachmentMemory; // the data of the image
        std::vector<VkImage> attachmentImages; // defines format of image
        std::vector<VkImageView> attachmentImageViews; // which part of image is relevant (Ex. mips)
        VkFramebuffer frameBuffer; // connects image to attachment

        //swapchain gets its own custom renderpass not in this class.

    };
}