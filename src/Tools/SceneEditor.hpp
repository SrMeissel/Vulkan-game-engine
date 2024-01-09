#pragma once

#include "../descriptorManager.hpp"
#include "../Pipeline/Renderer.hpp"

#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imgui_impl_vulkan.h"
#include "../../libs/imgui/imgui_impl_glfw.h"

#include <vector>
#include <array>

namespace engine {
    class SceneEditor {

        public:

            SceneEditor(Device& device, Window& window, Renderer& renderer);
            ~SceneEditor();

            void run(VkCommandBuffer commandBuffer);

            void configureViewport(VkImageView imageView, VkSampler sampler, VkExtent2D extent);

        private:
            Device& device;
            Window& window;
            Renderer& renderer;

            VkRenderPassCreateInfo* configureRenderPass();

            std::shared_ptr<DescriptorPool> imguiPool {};
            VkDescriptorSet viewportDescriptorSet;
            VkExtent2D viewportExtent;
    };
}