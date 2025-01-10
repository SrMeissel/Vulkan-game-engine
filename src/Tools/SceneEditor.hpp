#pragma once

#include "../descriptorManager.hpp"
#include "../Pipeline/Renderer.hpp"

#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imgui_impl_vulkan.h"
#include "../../libs/imgui/imgui_impl_glfw.h"

#include <vector>
#include <array>

class SceneEditor {
    public:

        SceneEditor(Window& window, renderer::Renderer& renderer);
        ~SceneEditor();

        void run(VkCommandBuffer commandBuffer);

        void configureViewport(VkImageView imageView, VkImageView secondaryView, VkSampler sampler, VkExtent2D extent);

    private:
        Window& window;
        renderer::Renderer& renderer;

        VkRenderPassCreateInfo* configureRenderPass();

        std::shared_ptr<renderer::DescriptorPool> imguiPool {};
        VkDescriptorSet viewportDescriptorSet;
        VkDescriptorSet secondaryViewportDescriptorSet;
        VkExtent2D viewportExtent;
};