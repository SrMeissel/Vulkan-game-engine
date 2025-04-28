#pragma once

#include "deviceManager.hpp"
#include "Utils.hpp"

#include <vector>

namespace renderer {
    class RenderPass {
        public:
            RenderPass(Device& device, VkRenderPassCreateInfo* info, VkExtent2D extent);
            RenderPass(Device& device, VkRenderPassCreateInfo* info, std::vector<AllocatedImage> images, VkExtent2D extent);
            ~RenderPass();

            // Not copyable or movable
	    // for some reason im sure
            RenderPass(const RenderPass &) = delete;
            RenderPass& operator=(const RenderPass &) = delete;
            RenderPass(RenderPass &&) = delete;
            RenderPass& operator=(RenderPass &&) = delete;

            VkExtent2D extent;
            float getAspectRatio() {return static_cast<float>(extent.width) / static_cast<float>(extent.height); }

	    VkRenderPass renderPass;
	    std::vector<AllocatedImage> images;
	    VkFramebuffer frameBuffer; // connects image to attachment
        private:
	    void createImageResources();
	    void createFrameBuffer();

	    Device& device;

	    VkRenderPassCreateInfo* info;
    };
}
