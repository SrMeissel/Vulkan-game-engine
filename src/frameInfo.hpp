#pragma once

#include "cameraManager.hpp"
#include "gameObject.hpp"

#include <vulkan/vulkan.h>

namespace engine {
    struct frameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        CameraManager& camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::map &gameObjects;
    };
}