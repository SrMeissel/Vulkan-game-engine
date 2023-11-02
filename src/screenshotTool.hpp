#pragma once

#include <vulkan/vulkan.h>
#include "Pipeline/deviceManager.hpp"

namespace engine {
    class ScreenshotTool {
        public:
            void takeScreenshot(VkImage srcImage, char* fileName, Device &device, VkExtent2D extent);
    };
}