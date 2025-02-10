#pragma once

#include "cameraManager.hpp"

#include <vulkan/vulkan.h>

namespace renderer {
    // struct frameInfo {
    //     int frameIndex;
    //     float frameTime;
    //     VkCommandBuffer commandBuffer;
    //     engine::CameraManager& camera;
    //     VkDescriptorSet globalDescriptorSet;
    // };

    struct GlobalUbo {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 inverseView{1.0f};
    };
}