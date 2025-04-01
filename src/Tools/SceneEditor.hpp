#pragma once

#include "../descriptorManager.hpp"
#include "../Pipeline/Renderer.hpp"

#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imgui_impl_vulkan.h"
#include "../../libs/imgui/imgui_impl_glfw.h"

#include "ECS/AssetManager.hpp"

#include "Tools/CameraControl.hpp"
#include "cameraManager.hpp"


#include <vector>
#include <array>

namespace editor {

    enum EngineState {
        RUNNING = 0,
        RESET = 1,
        PAUSED = 2
    };

    class SceneEditor {
        public:

        SceneEditor(Window& window, renderer::Renderer& renderer, ECS::AssetSystem& assetSystem);
        ~SceneEditor();

        void updateState(float dt, EngineState& state);
        void renderState(VkCommandBuffer commandBuffer);

        void configureViewport(VkImageView imageView, VkImageView secondaryView, VkSampler sampler, VkExtent2D extent);

        // ECS::Camera viewportCamera;
        // ECS::Transform viewportTransform{glm::vec3(0.0f, -3.5f, -12.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f)};
        ECS::Entity viewportEntity;

        private:
        Window& window;
        renderer::Renderer& renderer;
        ECS::AssetSystem& assetSystem;

        //VkRenderPassCreateInfo* configureRenderPass();

        std::shared_ptr<renderer::DescriptorPool> imguiPool {};
        VkDescriptorSet viewportDescriptorSet;
        VkDescriptorSet secondaryViewportDescriptorSet;
        VkExtent2D viewportExtent;
    };
}