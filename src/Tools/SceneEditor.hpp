#pragma once

#include "../descriptorManager.hpp"

#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imgui_impl_vulkan.h"
#include "../../libs/imgui/imgui_impl_glfw.h"

namespace engine {
    class SceneEditor {

        public:

            SceneEditor(Device& device, GLFWwindow* window, VkRenderPass renderPass);
            ~SceneEditor();

        private:
            Device& device;
            GLFWwindow* window;
            VkRenderPass& renderPass;

            std::shared_ptr<DescriptorPool> imguiPool {};
    };
}