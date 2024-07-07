#pragma once

#include "Pipeline/windowManager.hpp"
#include "Pipeline/deviceManager.hpp"
#include "Pipeline/Renderer.hpp"
#include "descriptorManager.hpp"
#include "screenshotTool.hpp"
#include "Components/textureManager.hpp"
#include "Tools/SceneEditor.hpp"

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"

#include "systems/ScriptingSystem.hpp"

#include "ECS/SavedataManager.hpp"

#include <memory>
#include <vector>

namespace engine {
    class app {
        public:
            static constexpr int WIDTH = 1200;
            static constexpr int HEIGHT = 800;

            app();
            ~app();

            app(const app &) = delete;
            app &operator=(const app &) = delete;

            void run();
        private:
            VkRenderPassCreateInfo* configureRenderPass();
            VkFormat chooseSwapSurfaceFormat();

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device device{window};
            Renderer renderer{window, device};

            SceneEditor sceneEditor{device, window, renderer};

            TextureManager textureManager{device}; 
            ScreenshotTool screenshotTool;

            std::shared_ptr<DescriptorPool> globalPool;
            
            ECS::AssetSystem assetSystem;
    };
}