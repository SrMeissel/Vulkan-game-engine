#pragma once

#include "cameraManager.hpp"

#include <vulkan/vulkan.h>

namespace engine {

    #define MAX_LIGHTS 10

    struct PointLight{
        glm::vec4 position{}; //ignore w
        glm::vec4 color{}; // w is intesnity
    };

    struct frameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        CameraManager& camera;
        VkDescriptorSet globalDescriptorSet;
    };

    struct GlobalUbo {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 inverseView{1.0f};
    };
}