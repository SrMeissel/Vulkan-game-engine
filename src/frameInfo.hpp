#pragma once

#include "cameraManager.hpp"
#include "gameObject.hpp"

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
        GameObject::map &gameObjects;
    };

    struct GlobalUbo {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 inverseView{1.0f};
    //glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});

    glm::vec4 ambientColor{1.0f, 1.0f, 1.0f, 0.02f};

    PointLight pointLights[MAX_LIGHTS];
    int numLights;
    };
}